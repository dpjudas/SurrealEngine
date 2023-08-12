
#include "Precomp.h"
#include "CollisionCommandlet.h"
#include "DebuggerApp.h"
#include "Engine.h"
#include "File.h"
#include "Collision/CollisionHash.h"

CollisionCommandlet::CollisionCommandlet()
{
	SetLongFormName("collision");
	SetShortDescription("Simple collision tester");
}

void CollisionCommandlet::OnCommand(DebuggerApp* console, const std::string& args)
{
	std::vector<std::string> params = SplitString(args);

	if (params.size() < 16)
		throw std::runtime_error("Invalid number of arguments");

	if (params[0] == "cylindercylindertrace")
	{
		dvec3 origin(std::stod(params[1]), std::stod(params[2]), std::stod(params[3]));
		dvec3 dir(std::stod(params[4]), std::stod(params[5]), std::stod(params[6]));
		double tmin = std::stod(params[7]);
		double tmax = std::stod(params[8]);
		dvec3 cCenterA(std::stod(params[9]), std::stod(params[10]), std::stod(params[11]));
		double cHeightA = std::stod(params[12]);
		double cRadiusA = std::stod(params[13]);
		double cHeightB = std::stod(params[14]);
		double cRadiusB = std::stod(params[15]);

		double t = CollisionHash::CylinderCylinderTrace(origin, dir, tmin, tmax, cCenterA, cHeightA, cRadiusA, cHeightB, cRadiusB);
		console->WriteOutput(std::to_string(t));
	}
}

void CollisionCommandlet::OnPrintHelp(DebuggerApp* console)
{
}
