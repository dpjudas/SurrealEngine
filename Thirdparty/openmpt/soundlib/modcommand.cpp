/*
 * modcommand.cpp
 * --------------
 * Purpose: Various functions for writing effects to patterns, converting ModCommands, etc.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "modcommand.h"
#include "mod_specifications.h"
#include "Sndfile.h"
#include "Tables.h"


OPENMPT_NAMESPACE_BEGIN


static constexpr EffectType effectTypes[] =
{
	EffectType::Normal, EffectType::Normal,  EffectType::Pitch,  EffectType::Pitch,
	EffectType::Pitch,  EffectType::Pitch,   EffectType::Volume, EffectType::Volume,
	EffectType::Volume, EffectType::Panning, EffectType::Normal, EffectType::Volume,
	EffectType::Global, EffectType::Volume,  EffectType::Global, EffectType::Normal,
	EffectType::Global, EffectType::Global,  EffectType::Normal, EffectType::Normal,
	EffectType::Normal, EffectType::Volume,  EffectType::Volume, EffectType::Global,
	EffectType::Global, EffectType::Normal,  EffectType::Pitch,  EffectType::Panning,
	EffectType::Pitch,  EffectType::Panning, EffectType::Normal, EffectType::Normal,
	EffectType::Normal, EffectType::Normal,  EffectType::Normal, EffectType::Pitch,
	EffectType::Pitch,  EffectType::Normal,  EffectType::Pitch,  EffectType::Pitch,
	EffectType::Pitch,  EffectType::Pitch,   EffectType::Normal, EffectType::Normal,
	EffectType::Normal, EffectType::Normal,  EffectType::Volume, EffectType::Normal,
	EffectType::Normal, EffectType::Volume,  EffectType::Pitch,  EffectType::Pitch,
	EffectType::Pitch,  EffectType::Pitch,   EffectType::Pitch,  EffectType::Pitch,
	EffectType::Volume, EffectType::Volume,
};

static_assert(std::size(effectTypes) == MAX_EFFECTS);


static constexpr EffectType volumeEffectTypes[] =
{
	EffectType::Normal, EffectType::Volume,  EffectType::Panning, EffectType::Volume,
	EffectType::Volume, EffectType::Volume,  EffectType::Volume,  EffectType::Pitch,
	EffectType::Pitch,  EffectType::Panning, EffectType::Panning, EffectType::Pitch,
	EffectType::Pitch,  EffectType::Pitch,   EffectType::Normal,  EffectType::Normal,
};

static_assert(std::size(volumeEffectTypes) == MAX_VOLCMDS);


EffectType ModCommand::GetEffectType(COMMAND cmd)
{
	if(cmd < std::size(effectTypes))
		return effectTypes[cmd];
	else
		return EffectType::Normal;
}


EffectType ModCommand::GetVolumeEffectType(VOLCMD volcmd)
{
	if(volcmd < std::size(volumeEffectTypes))
		return volumeEffectTypes[volcmd];
	else
		return EffectType::Normal;
}


// Convert an Exx command (MOD) to Sxx command (S3M)
void ModCommand::ExtendedMODtoS3MEffect()
{
	if(command != CMD_MODCMDEX)
		return;

	command = CMD_S3MCMDEX;
	switch(param & 0xF0)
	{
	case 0x00: command = CMD_NONE; break; // No filter control
	case 0x10: command = CMD_PORTAMENTOUP; param |= 0xF0; break;
	case 0x20: command = CMD_PORTAMENTODOWN; param |= 0xF0; break;
	case 0x30: param = (param & 0x0F) | 0x10; break;
	case 0x40: param = (param & 0x03) | 0x30; break;
	case 0x50: param = (param ^ 0x58) | 0x20; break;
	case 0x60: param = (param & 0x0F) | 0xB0; break;
	case 0x70: param = (param & 0x03) | 0x40; break;
	case 0x90: command = CMD_RETRIG; param = (param & 0x0F); break;
	case 0xA0: if(param & 0x0F) { command = CMD_VOLUMESLIDE; param = (param << 4) | 0x0F; } else command = CMD_NONE; break;
	case 0xB0: if(param & 0x0F) { command = CMD_VOLUMESLIDE; param = 0xF0 | static_cast<PARAM>(std::min(param & 0x0F, 0x0E)); } else command = CMD_NONE; break;
	case 0xC0: if(param == 0xC0) { command = CMD_NONE; note = NOTE_NOTECUT; } break;  // this does different things in IT and ST3
	case 0xD0: if(param == 0xD0) { command = CMD_NONE; } break;  // ditto
	// rest are the same or handled elsewhere
	}
}


// Convert an Sxx command (S3M) to Exx command (MOD)
void ModCommand::ExtendedS3MtoMODEffect()
{
	if(command != CMD_S3MCMDEX)
		return;

	command = CMD_MODCMDEX;
	switch(param & 0xF0)
	{
	case 0x10: param = (param & 0x0F) | 0x30; break;
	case 0x20: param = (param ^ 0x28) | 0x50; break;
	case 0x30: param = (param & 0x0F) | 0x40; break;
	case 0x40: param = (param & 0x0F) | 0x70; break;
	case 0x50: command = CMD_XFINEPORTAUPDOWN; break;  // map to unused X5x
	case 0x60: command = CMD_XFINEPORTAUPDOWN; break;  // map to unused X6x
	case 0x80: command = CMD_PANNING8; param = (param & 0x0F) * 0x11; break; // FT2 does actually not support E8x
	case 0x90: command = CMD_XFINEPORTAUPDOWN; break;  // map to unused X9x
	case 0xA0: command = CMD_XFINEPORTAUPDOWN; break;  // map to unused XAx
	case 0xB0: param = (param & 0x0F) | 0x60; break;
	case 0x70: command = CMD_NONE; break;  // No NNA / envelope control in MOD/XM format
	// rest are the same or handled elsewhere
	}
}


// Convert a mod command from one format to another.
void ModCommand::Convert(MODTYPE fromType, MODTYPE toType, const CSoundFile &sndFile)
{
	if(fromType == toType)
	{
		return;
	}

	if(fromType == MOD_TYPE_MTM)
	{
		// Special MTM fixups.
		// Retrigger with param 0
		if(command == CMD_MODCMDEX && param == 0x90)
		{
			command = CMD_NONE;
		} else if(command == CMD_VIBRATO)
		{
			// Vibrato is approximately half as deep compared to MOD/S3M.
			uint8 speed = (param & 0xF0);
			uint8 depth = (param & 0x0F) >> 1;
			param = speed | depth;
		}
		// Apart from these special fixups, do a regular conversion from MOD.
		fromType = MOD_TYPE_MOD;
	}
	if(command == CMD_DIGIREVERSESAMPLE && toType != MOD_TYPE_DIGI)
	{
		command = CMD_S3MCMDEX;
		param = 0x9F;
	}
	if(command == CMD_VOLUME8)
	{
		command = CMD_VOLUME;
		param = static_cast<PARAM>((param + 3u) / 4u);
	}

	// helper variables
	const bool oldTypeIsMOD = (fromType == MOD_TYPE_MOD), oldTypeIsXM = (fromType == MOD_TYPE_XM),
		oldTypeIsS3M = (fromType == MOD_TYPE_S3M), oldTypeIsIT = (fromType == MOD_TYPE_IT),
		oldTypeIsMPT = (fromType == MOD_TYPE_MPT), oldTypeIsMOD_XM = (oldTypeIsMOD || oldTypeIsXM),
		oldTypeIsS3M_IT_MPT = (oldTypeIsS3M || oldTypeIsIT || oldTypeIsMPT),
		oldTypeIsIT_MPT = (oldTypeIsIT || oldTypeIsMPT);

	const bool newTypeIsMOD = (toType == MOD_TYPE_MOD), newTypeIsXM =  (toType == MOD_TYPE_XM),
		newTypeIsS3M = (toType == MOD_TYPE_S3M), newTypeIsIT = (toType == MOD_TYPE_IT),
		newTypeIsMPT = (toType == MOD_TYPE_MPT), newTypeIsMOD_XM = (newTypeIsMOD || newTypeIsXM),
		newTypeIsS3M_IT_MPT = (newTypeIsS3M || newTypeIsIT || newTypeIsMPT),
		newTypeIsIT_MPT = (newTypeIsIT || newTypeIsMPT);

	const CModSpecifications &newSpecs = CSoundFile::GetModSpecifications(toType);

	//////////////////////////
	// Convert 8-bit Panning
	if(command == CMD_PANNING8)
	{
		if(newTypeIsS3M)
		{
			param = static_cast<uint8>((param + 1u) >> 1);
		} else if(oldTypeIsS3M)
		{
			if(param == 0xA4)
			{
				// surround remap
				command = (toType & (MOD_TYPE_IT | MOD_TYPE_MPT)) ? CMD_S3MCMDEX : CMD_XFINEPORTAUPDOWN;
				param = 0x91;
			} else
			{
				param = mpt::saturate_cast<PARAM>(param * 2u);
			}
		}
	} // End if(command == CMD_PANNING8)

	// Re-map \xx to Zxx if the new format only knows the latter command.
	if(command == CMD_SMOOTHMIDI && !newSpecs.HasCommand(CMD_SMOOTHMIDI) && newSpecs.HasCommand(CMD_MIDI))
	{
		command = CMD_MIDI;
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// MPTM to anything: Convert param control, extended envelope control, note delay+cut
	if(oldTypeIsMPT)
	{
		if(IsPcNote())
		{
			COMMAND newCmd = (note == NOTE_PC) ? CMD_MIDI : CMD_SMOOTHMIDI;
			if(!newSpecs.HasCommand(newCmd))
			{
				newCmd = CMD_MIDI;	// assuming that this was CMD_SMOOTHMIDI
				if(!newSpecs.HasCommand(newCmd))
				{
					newCmd = CMD_NONE;
				}
			}

			param = static_cast<PARAM>(std::min(static_cast<uint16>(maxColumnValue), GetValueEffectCol()) * 0x7F / maxColumnValue);
			command = newCmd; // might be removed later
			volcmd = VOLCMD_NONE;
			note = NOTE_NONE;
			instr = 0;
		}

		if((command == CMD_S3MCMDEX) && ((param & 0xF0) == 0x70) && ((param & 0x0F) > 0x0C))
		{
			// Extended pitch envelope control commands
			param = 0x7C;
		} else if(command == CMD_DELAYCUT)
		{
			command = CMD_S3MCMDEX;       // When converting to MOD/XM, this will be converted to CMD_MODCMDEX later
			param = 0xD0 | (param >> 4);  // Preserve delay nibble
		} else if(command == CMD_FINETUNE || command == CMD_FINETUNE_SMOOTH)
		{
			// Convert finetune from +/-128th of a semitone to (extra-)fine portamento (assumes linear slides, plus we're missing the actual pitch wheel depth of the instrument)
			if(param < 0x80)
			{
				command = CMD_PORTAMENTODOWN;
				param = 0x80 - param;
			} else if(param > 0x80)
			{
				command = CMD_PORTAMENTOUP;
				param -= 0x80;
			}
			if(param <= 30)
				param = static_cast<uint8>(0xE0u | ((param + 1u) / 2u));
			else
				param = static_cast<uint8>(0xF0u | std::min(static_cast<PARAM>((param + 7u) / 8u), PARAM(15)));
		}
	} // End if(oldTypeIsMPT)

	/////////////////////////////////////////
	// Convert MOD / XM to S3M / IT / MPTM
	if(!oldTypeIsS3M_IT_MPT && newTypeIsS3M_IT_MPT)
	{
		switch(command)
		{
		case CMD_ARPEGGIO:
			if(!param) command = CMD_NONE;	// 000 does nothing in MOD/XM
			break;

		case CMD_MODCMDEX:
			ExtendedMODtoS3MEffect();
			break;

		case CMD_VOLUME:
			// Effect column volume command overrides the volume column in XM.
			if(volcmd == VOLCMD_NONE || volcmd == VOLCMD_VOLUME)
			{
				volcmd = VOLCMD_VOLUME;
				vol = param;
				if(vol > 64) vol = 64;
				command = CMD_NONE;
				param = 0;
			} else if(volcmd == VOLCMD_PANNING)
			{
				std::swap(vol, param);
				volcmd = VOLCMD_VOLUME;
				if(vol > 64) vol = 64;
				command = CMD_S3MCMDEX;
				param = 0x80 | (param / 4);	// XM volcol panning is actually 4-Bit, so we can use 4-Bit panning here.
			}
			break;

		case CMD_PORTAMENTOUP:
			if(param > 0xDF) param = 0xDF;
			break;

		case CMD_PORTAMENTODOWN:
			if(param > 0xDF) param = 0xDF;
			break;

		case CMD_XFINEPORTAUPDOWN:
			switch(param & 0xF0)
			{
			case 0x10:	command = CMD_PORTAMENTOUP; param = (param & 0x0F) | 0xE0; break;
			case 0x20:	command = CMD_PORTAMENTODOWN; param = (param & 0x0F) | 0xE0; break;
			case 0x50:
			case 0x60:
			case 0x70:
			case 0x90:
			case 0xA0:
				command = CMD_S3MCMDEX;
				// Surround remap (this is the "official" command)
				if(toType & MOD_TYPE_S3M && param == 0x91)
				{
					command = CMD_PANNING8;
					param = 0xA4;
				}
				break;
			}
			break;

		case CMD_KEYOFF:
			if(note == NOTE_NONE)
			{
				note = newTypeIsS3M ? NOTE_NOTECUT : NOTE_KEYOFF;
				command = CMD_S3MCMDEX;
				if(param == 0)
					instr = 0;
				param = 0xD0 | std::min(param, PARAM(0x0F));
			}
			break;

		case CMD_PANNINGSLIDE:
			// swap L/R, convert to fine slide
			if(param & 0xF0)
			{
				param = 0xF0 | std::min(PARAM(0x0E), static_cast<PARAM>(param >> 4));
			} else
			{
				param = 0x0F | (std::min(PARAM(0x0E), static_cast<PARAM>(param & 0x0F)) << 4);
			}
			break;

		case CMD_DUMMY:
			// Command "W" in XM, equivalent implementation in IT / S3M is command " ".
			// Don't convert this command, as it overwrites effect memory in S3M in particular - a feature not supported by all S3M players.
			command = CMD_NONE;
			break;

		default:
			break;
		}
	} // End if(!oldTypeIsS3M_IT_MPT && newTypeIsS3M_IT_MPT)


	/////////////////////////////////////////
	// Convert S3M / IT / MPTM to MOD / XM
	else if(!oldTypeIsMOD_XM && newTypeIsMOD_XM)
	{
		if(note == NOTE_NOTECUT)
		{
			// convert note cut to C00 if possible or volume command otherwise (MOD/XM has no real way of cutting notes that cannot be "undone" by volume commands)
			note = NOTE_NONE;
			if(command == CMD_NONE || !newTypeIsXM)
			{
				command = CMD_VOLUME;
				param = 0;
			} else
			{
				volcmd = VOLCMD_VOLUME;
				vol = 0;
			}
		} else if(note == NOTE_FADE)
		{
			// convert note fade to note off
			note = NOTE_KEYOFF;
		}

		switch(command)
		{
		case CMD_S3MCMDEX:
			ExtendedS3MtoMODEffect();
			break;

		case CMD_TONEPORTAVOL:	// Can't do fine slides and portamento/vibrato at the same time :(
		case CMD_VIBRATOVOL:	// ditto
			if(volcmd == VOLCMD_NONE && (((param & 0xF0) && ((param & 0x0F) == 0x0F)) || ((param & 0x0F) && ((param & 0xF0) == 0xF0))))
			{
				// Try to salvage portamento/vibrato
				if(command == CMD_TONEPORTAVOL)
					volcmd = VOLCMD_TONEPORTAMENTO;
				else if(command == CMD_VIBRATOVOL)
					volcmd = VOLCMD_VIBRATODEPTH;
				vol = 0;
			}

		[[fallthrough]];
		case CMD_VOLUMESLIDE:
			if((param & 0xF0) && ((param & 0x0F) == 0x0F))
			{
				command = CMD_MODCMDEX;
				param = (param >> 4) | 0xA0;
			} else if((param & 0x0F) && ((param & 0xF0) == 0xF0))
			{
				command = CMD_MODCMDEX;
				param = (param & 0x0F) | 0xB0;
			}
			break;

		case CMD_PORTAMENTOUP:
			if(param >= 0xF0)
			{
				command = CMD_MODCMDEX;
				param = (param & 0x0F) | 0x10;
			} else if(param >= 0xE0)
			{
				if(newTypeIsXM)
				{
					command = CMD_XFINEPORTAUPDOWN;
					param = 0x10 | (param & 0x0F);
				} else
				{
					command = CMD_MODCMDEX;
					param = static_cast<uint8>((((param & 0x0Fu) + 3u) >> 2) | 0x10u);
				}
			} else
			{
				command = CMD_PORTAMENTOUP;
			}
			break;

		case CMD_PORTAMENTODOWN:
			if(param >= 0xF0)
			{
				command = CMD_MODCMDEX;
				param = (param & 0x0F) | 0x20;
			} else if(param >= 0xE0)
			{
				if(newTypeIsXM)
				{
					command = CMD_XFINEPORTAUPDOWN;
					param = 0x20 | (param & 0x0F);
				} else
				{
					command = CMD_MODCMDEX;
					param = static_cast<uint8>((((param & 0x0Fu) + 3u) >> 2) | 0x20u);
				}
			} else
			{
				command = CMD_PORTAMENTODOWN;
			}
			break;

		case CMD_TEMPO:
			if(param < 0x20) command = CMD_NONE; // no tempo slides
			break;

		case CMD_PANNINGSLIDE:
			// swap L/R, convert fine slides to normal slides
			if((param & 0x0F) == 0x0F && (param & 0xF0))
			{
				param = (param >> 4);
			} else if((param & 0xF0) == 0xF0 && (param & 0x0F))
			{
				param = (param & 0x0F) << 4;
			} else if(param & 0x0F)
			{
				param = 0xF0;
			} else if(param & 0xF0)
			{
				param = 0x0F;
			} else
			{
				param = 0;
			}
			break;

		case CMD_RETRIG:
			// Retrig: Q0y doesn't change volume in IT/S3M, but R0y in XM takes the last x parameter
			if(param != 0 && (param & 0xF0) == 0)
			{
				param |= 0x80;
			}
			break;

		default:
			break;
		}
	} // End if(!oldTypeIsMOD_XM && newTypeIsMOD_XM)


	///////////////////////
	// Convert IT to S3M
	else if(oldTypeIsIT_MPT && newTypeIsS3M)
	{
		if(note == NOTE_KEYOFF || note == NOTE_FADE)
			note = NOTE_NOTECUT;

		switch(command)
		{
		case CMD_S3MCMDEX:
			switch(param & 0xF0)
			{
			case 0x70: command = CMD_NONE; break;	// No NNA / envelope control in S3M format
			case 0x90:
				if(param == 0x91)
				{
					// surround remap (this is the "official" command)
					command = CMD_PANNING8;
					param = 0xA4;
				} else if(param == 0x90)
				{
					command = CMD_PANNING8;
					param = 0x40;
				}
				break;
			}
			break;

		case CMD_GLOBALVOLUME:
			param = static_cast<uint8>((std::min(PARAM(0x80), param) + 1u) / 2u);
			break;

		default:
			break;
		}
	} // End if(oldTypeIsIT_MPT && newTypeIsS3M)

	//////////////////////
	// Convert IT to XM
	if(oldTypeIsIT_MPT && newTypeIsXM)
	{
		switch(command)
		{
		case CMD_VIBRATO:
			// With linear slides, strength is roughly doubled.
			param = static_cast<uint8>((param & 0xF0u) | (((param & 0x0Fu) + 1u) / 2u));
			break;
		case CMD_GLOBALVOLUME:
			param = static_cast<uint8>((std::min(PARAM(0x80), param) + 1u) / 2u);
			break;
		default:
			break;
		}
	} // End if(oldTypeIsIT_MPT && newTypeIsXM)

	//////////////////////
	// Convert XM to IT
	if(oldTypeIsXM && newTypeIsIT_MPT)
	{
		switch(command)
		{
		case CMD_VIBRATO:
			// With linear slides, strength is roughly halved.
			param = (param & 0xF0) | std::min(static_cast<PARAM>((param & 0x0F) * 2u), PARAM(15));
			break;
		case CMD_GLOBALVOLUME:
			param = std::min(PARAM(0x40), param) * 2u;
			break;
		default:
			break;
		}
	} // End if(oldTypeIsIT_MPT && newTypeIsXM)

	///////////////////////////////////
	// MOD / XM Speed/Tempo limits
	if(newTypeIsMOD_XM)
	{
		switch(command)
		{
		case CMD_SPEED:
			param = std::min(param, PARAM(0x1F));
			break;
			break;
		case CMD_TEMPO:
			param = std::max(param, PARAM(0x20));
			break;
		default:
			break;
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Convert MOD to anything - adjust effect memory, remove Invert Loop
	if(oldTypeIsMOD)
	{
		switch(command)
		{
		case CMD_TONEPORTAVOL:  // lacks memory -> 500 is the same as 300
			if(param == 0x00)
				command = CMD_TONEPORTAMENTO;
			break;

		case CMD_VIBRATOVOL:  // lacks memory -> 600 is the same as 400
			if(param == 0x00)
				command = CMD_VIBRATO;
			break;

		case CMD_PORTAMENTOUP:  // lacks memory -> remove
		case CMD_PORTAMENTODOWN:
		case CMD_VOLUMESLIDE:
			if(param == 0x00)
				command = CMD_NONE;
			break;

		case CMD_MODCMDEX:
			// No effect memory
			if(param == 0x10 || param == 0x20 || param == 0xA0 || param == 0xB0)
				command = CMD_NONE;
			// This would turn into "Set Active Macro", so let's better remove it
			if((param & 0xF0) == 0xF0)
				command = CMD_NONE;
			break;

		case CMD_S3MCMDEX:
			if((param & 0xF0) == 0xF0)
				command = CMD_NONE;
			break;
		default:
			break;
		}
	} // End if(oldTypeIsMOD && newTypeIsXM)

	/////////////////////////////////////////////////////////////////////
	// Convert anything to MOD - remove volume column, remove Set Macro
	if(newTypeIsMOD)
	{
		// convert note off events
		if(IsSpecialNote())
		{
			note = NOTE_NONE;
			// no effect present, so just convert note off to volume 0
			if(command == CMD_NONE)
			{
				command = CMD_VOLUME;
				param = 0;
				// EDx effect present, so convert it to ECx
			} else if((command == CMD_MODCMDEX) && ((param & 0xF0) == 0xD0))
			{
				param = 0xC0 | (param & 0x0F);
			}
		}

		if(command != CMD_NONE) switch(command)
		{
			case CMD_RETRIG: // MOD only has E9x
				command = CMD_MODCMDEX;
				param = 0x90 | (param & 0x0F);
				break;

			case CMD_MODCMDEX: // This would turn into "Invert Loop", so let's better remove it
				if((param & 0xF0) == 0xF0) command = CMD_NONE;
				break;

			default:
				break;
		}

		if(command == CMD_NONE) switch(volcmd)
		{
			case VOLCMD_VOLUME:
				command = CMD_VOLUME;
				param = vol;
				break;

			case VOLCMD_PANNING:
				command = CMD_PANNING8;
				param = vol < 64 ? vol << 2 : 255;
				break;

			case VOLCMD_VOLSLIDEDOWN:
				command = CMD_VOLUMESLIDE;
				param = vol;
				break;

			case VOLCMD_VOLSLIDEUP:
				command = CMD_VOLUMESLIDE;
				param = vol << 4;
				break;

			case VOLCMD_FINEVOLDOWN:
				command = CMD_MODCMDEX;
				param = 0xB0 | vol;
				break;

			case VOLCMD_FINEVOLUP:
				command = CMD_MODCMDEX;
				param = 0xA0 | vol;
				break;

			case VOLCMD_PORTADOWN:
				command = CMD_PORTAMENTODOWN;
				param = vol << 2;
				break;

			case VOLCMD_PORTAUP:
				command = CMD_PORTAMENTOUP;
				param = vol << 2;
				break;

			case VOLCMD_TONEPORTAMENTO:
				command = CMD_TONEPORTAMENTO;
				param = vol << 2;
				break;

			case VOLCMD_VIBRATODEPTH:
				command = CMD_VIBRATO;
				param = vol;
				break;

			case VOLCMD_VIBRATOSPEED:
				command = CMD_VIBRATO;
				param = vol << 4;
				break;

			default:
				break;
		}
		volcmd = VOLCMD_NONE;
	} // End if(newTypeIsMOD)

	///////////////////////////////////////////////////
	// Convert anything to S3M - adjust volume column
	if(newTypeIsS3M)
	{
		if(command == CMD_NONE) switch(volcmd)
		{
			case VOLCMD_VOLSLIDEDOWN:
				command = CMD_VOLUMESLIDE;
				param = vol;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_VOLSLIDEUP:
				command = CMD_VOLUMESLIDE;
				param = vol << 4;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_FINEVOLDOWN:
				command = CMD_VOLUMESLIDE;
				param = 0xF0 | vol;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_FINEVOLUP:
				command = CMD_VOLUMESLIDE;
				param = (vol << 4) | 0x0F;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_PORTADOWN:
				command = CMD_PORTAMENTODOWN;
				param = vol << 2;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_PORTAUP:
				command = CMD_PORTAMENTOUP;
				param = vol << 2;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_TONEPORTAMENTO:
				command = CMD_TONEPORTAMENTO;
				param = vol << 2;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_VIBRATODEPTH:
				command = CMD_VIBRATO;
				param = vol;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_VIBRATOSPEED:
				command = CMD_VIBRATO;
				param = vol << 4;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_PANSLIDELEFT:
				command = CMD_PANNINGSLIDE;
				param = vol << 4;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_PANSLIDERIGHT:
				command = CMD_PANNINGSLIDE;
				param = vol;
				volcmd = VOLCMD_NONE;
				break;

			default:
				break;
		}
	} // End if(newTypeIsS3M)

	////////////////////////////////////////////////////////////////////////
	// Convert anything to XM - adjust volume column, breaking EDx command
	if(newTypeIsXM)
	{
		// remove EDx if no note is next to it, or it will retrigger the note in FT2 mode
		if(command == CMD_MODCMDEX && (param & 0xF0) == 0xD0 && note == NOTE_NONE)
		{
			command = CMD_NONE;
			param = 0;
		}

		if(IsSpecialNote())
		{
			// Instrument numbers next to Note Off reset instrument settings
			instr = 0;

			if(command == CMD_MODCMDEX && (param & 0xF0) == 0xD0)
			{
				// Note Off + Note Delay does nothing when using envelopes.
				note = NOTE_NONE;
				command = CMD_KEYOFF;
				param &= 0x0F;
			}
		}

		// Convert some commands which behave differently or don't exist
		if(command == CMD_NONE) switch(volcmd)
		{
			case VOLCMD_PORTADOWN:
				command = CMD_PORTAMENTODOWN;
				param = vol << 2;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_PORTAUP:
				command = CMD_PORTAMENTOUP;
				param = vol << 2;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_TONEPORTAMENTO:
				command = CMD_TONEPORTAMENTO;
				param = ImpulseTrackerPortaVolCmd[vol & 0x0F];
				volcmd = VOLCMD_NONE;
				break;

			default:
				break;
		}
	} // End if(newTypeIsXM)

	///////////////////////////////////////////////////
	// Convert anything to IT - adjust volume column
	if(newTypeIsIT_MPT)
	{
		// Convert some commands which behave differently or don't exist
		if(!oldTypeIsIT_MPT && command == CMD_NONE) switch(volcmd)
		{
			case VOLCMD_PANSLIDELEFT:
				command = CMD_PANNINGSLIDE;
				param = vol << 4;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_PANSLIDERIGHT:
				command = CMD_PANNINGSLIDE;
				param = vol;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_VIBRATOSPEED:
				command = CMD_VIBRATO;
				param = vol << 4;
				volcmd = VOLCMD_NONE;
				break;

			case VOLCMD_TONEPORTAMENTO:
				command = CMD_TONEPORTAMENTO;
				param = vol << 4;
				volcmd = VOLCMD_NONE;
				break;

			default:
				break;
		}

		switch(volcmd)
		{
		case VOLCMD_VOLSLIDEDOWN:
		case VOLCMD_VOLSLIDEUP:
		case VOLCMD_FINEVOLDOWN:
		case VOLCMD_FINEVOLUP:
		case VOLCMD_PORTADOWN:
		case VOLCMD_PORTAUP:
		case VOLCMD_TONEPORTAMENTO:
		case VOLCMD_VIBRATODEPTH:
			// OpenMPT-specific commands
		case VOLCMD_OFFSET:
			vol = std::min(vol, VOL(9));
			break;
		default:
			break;
		}
	} // End if(newTypeIsIT_MPT)

	// Fix volume column offset for formats that don't have it.
	if(volcmd == VOLCMD_OFFSET && !newSpecs.HasVolCommand(VOLCMD_OFFSET) && (command == CMD_NONE || command == CMD_OFFSET || !newSpecs.HasCommand(command)))
	{
		const ModCommand::PARAM oldOffset = (command == CMD_OFFSET) ? param : 0;
		command = CMD_OFFSET;
		volcmd = VOLCMD_NONE;
		SAMPLEINDEX smp = instr;
		if(smp > 0 && smp <= sndFile.GetNumInstruments() && IsNote() && sndFile.Instruments[smp] != nullptr)
			smp = sndFile.Instruments[smp]->Keyboard[note - NOTE_MIN];

		if(smp > 0 && smp <= sndFile.GetNumSamples() && vol <= std::size(ModSample().cues))
		{
			const ModSample &sample = sndFile.GetSample(smp);
			if(vol == 0)
				param = mpt::saturate_cast<ModCommand::PARAM>(Util::muldivr_unsigned(sample.nLength, oldOffset, 65536u));
			else
				param = mpt::saturate_cast<ModCommand::PARAM>((sample.cues[vol - 1] + (oldOffset * 256u) + 128u) / 256u);
		} else
		{
			param = vol << 3;
		}
	} else if(volcmd == VOLCMD_PLAYCONTROL && (vol == 2 || vol == 3) && command == CMD_NONE
		&& !newSpecs.HasVolCommand(VOLCMD_PLAYCONTROL)
		&& (newSpecs.HasCommand(CMD_S3MCMDEX) || newSpecs.HasCommand(CMD_XFINEPORTAUPDOWN)))
	{
		volcmd = VOLCMD_NONE;
		param = vol - 2 + 0x9E;
		if(newSpecs.HasCommand(CMD_S3MCMDEX))
			command = CMD_S3MCMDEX;
		else
			command = CMD_XFINEPORTAUPDOWN;
	}

	// Offset effect memory is only updated when the command is placed next to a note.
	if(oldTypeIsXM && command == CMD_OFFSET && !IsNote())
		command = CMD_NONE;

	if((command == CMD_REVERSEOFFSET || command == CMD_OFFSETPERCENTAGE) && !newSpecs.HasCommand(command))
	{
		command = CMD_OFFSET;
	} else if(command == CMD_HMN_MEGA_ARP && !newSpecs.HasCommand(CMD_HMN_MEGA_ARP))
	{
		command = CMD_ARPEGGIO;
		param = (HisMastersNoiseMegaArp[param & 0x0F][1] << 4) | HisMastersNoiseMegaArp[param & 0x0F][2];
	}

	if(!newSpecs.HasNote(note))
		note = NOTE_NONE;

	// ensure the commands really exist in this format
	if(!newSpecs.HasCommand(command))
		command = CMD_NONE;
	if(!newSpecs.HasVolCommand(volcmd))
		volcmd = VOLCMD_NONE;

}


bool ModCommand::IsAnyPitchSlide() const
{
	switch(command)
	{
	case CMD_PORTAMENTOUP:
	case CMD_PORTAMENTODOWN:
	case CMD_TONEPORTAMENTO:
	case CMD_TONEPORTAVOL:
	case CMD_NOTESLIDEUP:
	case CMD_NOTESLIDEDOWN:
	case CMD_NOTESLIDEUPRETRIG:
	case CMD_NOTESLIDEDOWNRETRIG:
	case CMD_AUTO_PORTAUP:
	case CMD_AUTO_PORTADOWN:
	case CMD_AUTO_PORTAUP_FINE:
	case CMD_AUTO_PORTADOWN_FINE:
	case CMD_AUTO_PORTAMENTO_FC:
	case CMD_TONEPORTA_DURATION:
		return true;
	case CMD_MODCMDEX:
	case CMD_XFINEPORTAUPDOWN:
		if(param >= 0x10 && param <= 0x2F)
			return true;
		break;
	default:
		break;
	}
	switch(volcmd)
	{
	case VOLCMD_TONEPORTAMENTO:
	case VOLCMD_PORTAUP:
	case VOLCMD_PORTADOWN:
		return true;
	default:
		break;
	}
	return false;
}


bool ModCommand::IsContinousCommand(const CSoundFile &sndFile) const
{
	switch(command)
	{
	case CMD_ARPEGGIO:
	case CMD_TONEPORTAMENTO:
	case CMD_VIBRATO:
	case CMD_TREMOLO:
	case CMD_RETRIG:
	case CMD_TREMOR:
	case CMD_FINEVIBRATO:
	case CMD_PANBRELLO:
	case CMD_SMOOTHMIDI:
	case CMD_NOTESLIDEUP:
	case CMD_NOTESLIDEDOWN:
	case CMD_NOTESLIDEUPRETRIG:
	case CMD_NOTESLIDEDOWNRETRIG:
	case CMD_HMN_MEGA_ARP:
	case CMD_AUTO_VOLUMESLIDE:
	case CMD_AUTO_PORTAUP:
	case CMD_AUTO_PORTADOWN:
	case CMD_AUTO_PORTAUP_FINE:
	case CMD_AUTO_PORTADOWN_FINE:
	case CMD_AUTO_PORTAMENTO_FC:
		return true;
	case CMD_PORTAMENTOUP:
	case CMD_PORTAMENTODOWN:
		if(!param && sndFile.GetType() == MOD_TYPE_MOD)
			return false;
		if(sndFile.GetType() & (MOD_TYPE_MOD | MOD_TYPE_XM | MOD_TYPE_MT2 | MOD_TYPE_MED | MOD_TYPE_AMF0 | MOD_TYPE_DIGI | MOD_TYPE_STP | MOD_TYPE_DTM))
			return true;
		if(param >= 0xF0)
			return false;
		if(param >= 0xE0 && sndFile.GetType() != MOD_TYPE_DBM)
			return false;
		return true;
	case CMD_VOLUMESLIDE:
	case CMD_TONEPORTAVOL:
	case CMD_VIBRATOVOL:
	case CMD_GLOBALVOLSLIDE:
	case CMD_CHANNELVOLSLIDE:
	case CMD_PANNINGSLIDE:
		if(!param && sndFile.GetType() == MOD_TYPE_MOD)
			return false;
		if(sndFile.GetType() & (MOD_TYPE_MOD | MOD_TYPE_XM | MOD_TYPE_AMF0 | MOD_TYPE_MED | MOD_TYPE_DIGI))
			return true;
		if((param & 0xF0) == 0xF0 && (param & 0x0F))
			return false;
		if((param & 0x0F) == 0x0F && (param & 0xF0))
			return false;
		return true;
	case CMD_TEMPO:
		return (param < 0x20);
	default:
		return false;
	}
}


bool ModCommand::IsContinousVolColCommand() const
{
	switch(volcmd)
	{
	case VOLCMD_VOLSLIDEUP:
	case VOLCMD_VOLSLIDEDOWN:
	case VOLCMD_VIBRATOSPEED:
	case VOLCMD_VIBRATODEPTH:
	case VOLCMD_PANSLIDELEFT:
	case VOLCMD_PANSLIDERIGHT:
	case VOLCMD_TONEPORTAMENTO:
	case VOLCMD_PORTAUP:
	case VOLCMD_PORTADOWN:
		return true;
	default:
		return false;
	}
}


bool ModCommand::IsSlideUpDownCommand() const
{
	switch(command)
	{
		case CMD_VOLUMESLIDE:
		case CMD_TONEPORTAVOL:
		case CMD_VIBRATOVOL:
		case CMD_GLOBALVOLSLIDE:
		case CMD_CHANNELVOLSLIDE:
		case CMD_PANNINGSLIDE:
		case CMD_AUTO_VOLUMESLIDE:
			return true;
		default:
			return false;
	}
}


bool ModCommand::IsGlobalCommand(COMMAND command, PARAM param)
{
	switch(command)
	{
	case CMD_POSITIONJUMP:
	case CMD_PATTERNBREAK:
	case CMD_SPEED:
	case CMD_TEMPO:
	case CMD_GLOBALVOLUME:
	case CMD_GLOBALVOLSLIDE:
	case CMD_MIDI:
	case CMD_SMOOTHMIDI:
	case CMD_DBMECHO:
		return true;
	case CMD_MODCMDEX:
		switch(param & 0xF0)
		{
		case 0x00:	// LED Filter
		case 0x60:	// Pattern Loop
		case 0xE0:	// Row Delay
			return true;
		default:
			return false;
		}
	case CMD_XFINEPORTAUPDOWN:
	case CMD_S3MCMDEX:
		switch(param & 0xF0)
		{
		case 0x60:	// Tick Delay
		case 0x90:	// Sound Control
		case 0xB0:	// Pattern Loop
		case 0xE0:	// Row Delay
			return true;
		default:
			return false;
		}

	default:
		return false;
	}
}


bool ModCommand::CommandHasTwoNibbles(COMMAND command)
{
	switch(command)
	{
	case CMD_ARPEGGIO:
	case CMD_VIBRATO:
	case CMD_TONEPORTAVOL:
	case CMD_VIBRATOVOL:
	case CMD_TREMOLO:
	case CMD_VOLUMESLIDE:
	case CMD_RETRIG:
	case CMD_TREMOR:
	case CMD_MODCMDEX:
	case CMD_S3MCMDEX:
	case CMD_CHANNELVOLSLIDE:
	case CMD_GLOBALVOLSLIDE:
	case CMD_FINEVIBRATO:
	case CMD_PANBRELLO:
	case CMD_XFINEPORTAUPDOWN:
	case CMD_PANNINGSLIDE:
	case CMD_DELAYCUT:
	case CMD_NOTESLIDEUP:
	case CMD_NOTESLIDEDOWN:
	case CMD_NOTESLIDEUPRETRIG:
	case CMD_NOTESLIDEDOWNRETRIG:
	case CMD_AUTO_VOLUMESLIDE:
		return true;
	default:
		return false;
	}
}


// "Importance" of every FX command. Table is used for importing from formats with multiple effect colums
// and is approximately the same as in SchismTracker.
size_t ModCommand::GetEffectWeight(COMMAND cmd)
{
	// Effect weights, sorted from lowest to highest weight.
	static constexpr COMMAND weights[] =
	{
		CMD_NONE,
		CMD_DUMMY,
		CMD_XPARAM,
		CMD_SETENVPOSITION,
		CMD_MED_SYNTH_JUMP,
		CMD_KEYOFF,
		CMD_TREMOLO,
		CMD_FINEVIBRATO,
		CMD_VIBRATO,
		CMD_XFINEPORTAUPDOWN,
		CMD_FINETUNE,
		CMD_FINETUNE_SMOOTH,
		CMD_PANBRELLO,
		CMD_S3MCMDEX,
		CMD_MODCMDEX,
		CMD_DELAYCUT,
		CMD_MIDI,
		CMD_SMOOTHMIDI,
		CMD_PANNINGSLIDE,
		CMD_PANNING8,
		CMD_NOTESLIDEUPRETRIG,
		CMD_NOTESLIDEUP,
		CMD_NOTESLIDEDOWNRETRIG,
		CMD_NOTESLIDEDOWN,
		CMD_PORTAMENTOUP,
		CMD_AUTO_PORTAMENTO_FC,
		CMD_AUTO_PORTAUP_FINE,
		CMD_AUTO_PORTAUP,
		CMD_PORTAMENTODOWN,
		CMD_AUTO_PORTADOWN_FINE,
		CMD_AUTO_PORTADOWN,
		CMD_VOLUMESLIDE,
		CMD_AUTO_VOLUMESLIDE,
		CMD_VIBRATOVOL,
		CMD_VOLUME,
		CMD_VOLUME8,
		CMD_DIGIREVERSESAMPLE,
		CMD_REVERSEOFFSET,
		CMD_OFFSETPERCENTAGE,
		CMD_OFFSET,
		CMD_TREMOR,
		CMD_RETRIG,
		CMD_HMN_MEGA_ARP,
		CMD_ARPEGGIO,
		CMD_TONEPORTA_DURATION,
		CMD_TONEPORTAMENTO,
		CMD_TONEPORTAVOL,
		CMD_DBMECHO,
		CMD_VOLUMEDOWN_DURATION,
		CMD_VOLUMEDOWN_ETX,
		CMD_CHANNELVOLSLIDE,
		CMD_CHANNELVOLUME,
		CMD_GLOBALVOLSLIDE,
		CMD_GLOBALVOLUME,
		CMD_TEMPO,
		CMD_SPEED,
		CMD_POSITIONJUMP,
		CMD_PATTERNBREAK,
	};
	static_assert(std::size(weights) == MAX_EFFECTS);

	for(size_t i = 0; i < std::size(weights); i++)
	{
		if(weights[i] == cmd)
		{
			return i;
		}
	}
	// Invalid / unknown command.
	MPT_ASSERT_NOTREACHED();
	return 0;
}


// Try to convert a fx column command into a volume column command.
// Returns the translated command if successful.
// Some commands can only be converted by losing some precision.
// If moving the command into the volume column is more important than accuracy, use force = true.
// (Code translated from SchismTracker and mainly supposed to be used with loaders ported from this tracker)
std::pair<VolumeCommand, ModCommand::VOL> ModCommand::ConvertToVolCommand(const EffectCommand effect, ModCommand::PARAM param, bool force)
{
	switch(effect)
	{
	case CMD_NONE:
		break;
	case CMD_VOLUME:
		return {VOLCMD_VOLUME, std::min(param, PARAM(64))};
	case CMD_VOLUME8:
		if(!force && (param & 3))
			break;
		return {VOLCMD_VOLUME, static_cast<VOL>((param + 3u) / 4u)};
	case CMD_PORTAMENTOUP:
		// if not force, reject when dividing causes loss of data in LSB, or if the final value is too
		// large to fit. (volume column Ex/Fx are four times stronger than effect column)
		if(!force && ((param & 3) || param >= 0xE0))
			break;
		return {VOLCMD_PORTAUP, static_cast<VOL>(param / 4u)};
	case CMD_PORTAMENTODOWN:
		if(!force && ((param & 3) || param >= 0xE0))
			break;
		return {VOLCMD_PORTADOWN, static_cast<VOL>(param / 4u)};
	case CMD_TONEPORTAMENTO:
		if(param >= 0xF0)
		{
			// hack for people who can't type F twice :)
			return {VOLCMD_TONEPORTAMENTO, VOL(9)};
		}
		for(uint8 n = 0; n < 10; n++)
		{
			if(force
				? (param <= ImpulseTrackerPortaVolCmd[n])
				: (param == ImpulseTrackerPortaVolCmd[n]))
			{
				return {VOLCMD_TONEPORTAMENTO, n};
			}
		}
		break;
	case CMD_VIBRATO:
		if(force)
			param = std::min(static_cast<PARAM>(param & 0x0F), PARAM(9));
		else if((param & 0x0F) > 9 || (param & 0xF0) != 0)
			break;
		return {VOLCMD_VIBRATODEPTH, static_cast<VOL>(param & 0x0F)};
	case CMD_FINEVIBRATO:
		if(force)
			param = 0;
		else if(param)
			break;
		return {VOLCMD_VIBRATODEPTH, param};
	case CMD_PANNING8:
		if(param == 255)
			param = 64;
		else
			param /= 4;
		return {VOLCMD_PANNING, param};
	case CMD_VOLUMESLIDE:
		if(param == 0)
			break;
		if((param & 0x0F) == 0)  // Dx0 / Cx
			return {VOLCMD_VOLSLIDEUP, static_cast<VOL>(param >> 4)};
		else if((param & 0xF0) == 0)  // D0x / Dx
			return {VOLCMD_VOLSLIDEDOWN, param};
		else if((param & 0x0F) == 0x0F)  // DxF / Ax
			return {VOLCMD_FINEVOLUP, static_cast<VOL>(param >> 4)};
		else if((param & 0xF0) == 0xF0)  // DFx / Bx
			return {VOLCMD_FINEVOLDOWN, static_cast<VOL>(param & 0x0F)};
		break;
	case CMD_S3MCMDEX:
		switch(param & 0xF0)
		{
		case 0x80:
			return {VOLCMD_PANNING, static_cast<VOL>(((param & 0x0F) << 2) + 2)};
		case 0x90:
			if(param >= 0x9E && force)
				return {VOLCMD_PLAYCONTROL, static_cast<VOL>(param - 0x9E + 2)};
			break;
		default:
			break;
		}
		break;
	case CMD_MODCMDEX:
		switch(param & 0xF0)
		{
			case 0x80:
				return {VOLCMD_PANNING, static_cast<VOL>(((param & 0x0F) << 2) + 2)};
			case 0xA0:
				return {VOLCMD_FINEVOLUP, static_cast<VOL>(param & 0x0F)};
			case 0xB0:
				return {VOLCMD_FINEVOLDOWN, static_cast<VOL>(param & 0x0F)};
			default:
				break;
		}
		break;
	default:
		break;
	}
	return {VOLCMD_NONE, VOL(0)};
}

// Try to combine two commands into one. Returns true on success and the combined command is placed in eff1 / param1.
bool ModCommand::CombineEffects(EffectCommand &eff1, uint8 &param1, EffectCommand &eff2, uint8 &param2)
{
	if(eff1 == CMD_VOLUMESLIDE && (eff2 == CMD_VIBRATO || eff2 == CMD_TONEPORTAVOL) && param2 == 0)
	{
		// Merge commands
		if(eff2 == CMD_VIBRATO)
		{
			eff1 = CMD_VIBRATOVOL;
		} else
		{
			eff1 = CMD_TONEPORTAVOL;
		}
		eff2 = CMD_NONE;
		return true;
	} else if(eff2 == CMD_VOLUMESLIDE && (eff1 == CMD_VIBRATO || eff1 == CMD_TONEPORTAVOL) && param1 == 0)
	{
		// Merge commands
		if(eff1 == CMD_VIBRATO)
		{
			eff1 = CMD_VIBRATOVOL;
		} else
		{
			eff1 = CMD_TONEPORTAVOL;
		}
		param1 = param2;
		eff2 = CMD_NONE;
		return true;
	} else if(eff1 == CMD_OFFSET && eff2 == CMD_S3MCMDEX && param2 == 0x9F)
	{
		// Reverse offset
		eff1 = CMD_REVERSEOFFSET;
		eff2 = CMD_NONE;
		return true;
	} else if(eff1 == CMD_S3MCMDEX && param1 == 0x9F && eff2 == CMD_OFFSET)
	{
		// Reverse offset
		eff1 = CMD_REVERSEOFFSET;
		param1 = param2;
		eff2 = CMD_NONE;
		return true;
	} else
	{
		return false;
	}
}


std::pair<EffectCommand, ModCommand::PARAM> ModCommand::FillInTwoCommands(EffectCommand effect1, uint8 param1, EffectCommand effect2, uint8 param2, bool allowLowResOffset)
{
	if(effect1 == effect2)
	{
		// For non-sliding, absolute effects, it doesn't make sense to keep both commands
		switch(effect1)
		{
		case CMD_ARPEGGIO:
		case CMD_PANNING8:
		case CMD_OFFSET:
		case CMD_POSITIONJUMP:
		case CMD_VOLUME:
		case CMD_PATTERNBREAK:
		case CMD_SPEED:
		case CMD_TEMPO:
		case CMD_CHANNELVOLUME:
		case CMD_GLOBALVOLUME:
		case CMD_KEYOFF:
		case CMD_SETENVPOSITION:
		case CMD_MIDI:
		case CMD_SMOOTHMIDI:
		case CMD_DELAYCUT:
		case CMD_FINETUNE:
		case CMD_FINETUNE_SMOOTH:
		case CMD_DUMMY:
		case CMD_REVERSEOFFSET:
		case CMD_DBMECHO:
		case CMD_OFFSETPERCENTAGE:
		case CMD_DIGIREVERSESAMPLE:
		case CMD_VOLUME8:
		case CMD_HMN_MEGA_ARP:
		case CMD_MED_SYNTH_JUMP:
			effect2 = CMD_NONE;
			break;
		default:
			break;
		}
	}

	for(uint8 n = 0; n < 4; n++)
	{
		if(auto volCmd = ModCommand::ConvertToVolCommand(effect1, param1, (n > 1)); effect1 == CMD_NONE || volCmd.first != VOLCMD_NONE)
		{
			SetVolumeCommand(volCmd);
			SetEffectCommand(effect2, param2);
			return {CMD_NONE, ModCommand::PARAM(0)};
		}
		std::swap(effect1, effect2);
		std::swap(param1, param2);
	}

	// Can only keep one command :(
	if(GetEffectWeight(effect1) > GetEffectWeight(effect2))
	{
		std::swap(effect1, effect2);
		std::swap(param1, param2);
	}
	if(effect2 == CMD_OFFSET && (allowLowResOffset || param2 == 0))
	{
		SetVolumeCommand(VOLCMD_OFFSET, static_cast<ModCommand::VOL>(param2 ? std::max(param2 * 9 / 255, 1) : 0));
		SetEffectCommand(effect1, param1);
		return {CMD_NONE, ModCommand::PARAM(0)};
	}
	SetVolumeCommand(VOLCMD_NONE, 0);
	SetEffectCommand(effect2, param2);
	return {effect1, param1};
}


OPENMPT_NAMESPACE_END
