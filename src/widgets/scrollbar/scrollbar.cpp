
#include "widgets/scrollbar/scrollbar.h"
#include "core/colorf.h"
#include <stdexcept>

Scrollbar::Scrollbar(Widget* parent) : Widget(parent)
{
	SetStyleClass("scrollbar");
	UpdatePartPositions();

	mouse_down_timer = new Timer(this);
	mouse_down_timer->FuncExpired = [this]() { OnTimerExpired(); };
}

Scrollbar::~Scrollbar()
{
}

bool Scrollbar::IsVertical() const
{
	return vertical;
}

bool Scrollbar::IsHorizontal() const
{
	return !vertical;
}

double Scrollbar::GetMin() const
{
	return scroll_min;
}

double Scrollbar::GetMax() const
{
	return scroll_max;
}

double Scrollbar::GetLineStep() const
{
	return line_step;
}

double Scrollbar::GetPageStep() const
{
	return page_step;
}

double Scrollbar::GetPosition() const
{
	return position;
}

void Scrollbar::SetVertical()
{
	vertical = true;
	if (UpdatePartPositions())
		Update();
}

void Scrollbar::SetHorizontal()
{
	vertical = false;
	if (UpdatePartPositions())
		Update();
}

void Scrollbar::SetMin(double new_scroll_min)
{
	SetRanges(new_scroll_min, scroll_max, line_step, page_step);
}

void Scrollbar::SetMax(double new_scroll_max)
{
	SetRanges(scroll_min, new_scroll_max, line_step, page_step);
}

void Scrollbar::SetLineStep(double step)
{
	SetRanges(scroll_min, scroll_max, step, page_step);
}

void Scrollbar::SetPageStep(double step)
{
	SetRanges(scroll_min, scroll_max, line_step, step);
}

void Scrollbar::SetRanges(double new_scroll_min, double new_scroll_max, double new_line_step, double new_page_step)
{
	if (new_scroll_min >= new_scroll_max || new_line_step <= 0.0 || new_page_step <= 0.0)
		throw std::runtime_error("Scrollbar ranges out of bounds!");
	scroll_min = new_scroll_min;
	scroll_max = new_scroll_max;
	line_step = new_line_step;
	page_step = new_page_step;
	if (position >= scroll_max)
		position = scroll_max - 1.0;
	if (position < scroll_min)
		position = scroll_min;
	if (UpdatePartPositions())
		Update();
}

void Scrollbar::SetRanges(double view_size, double total_size)
{
	if (view_size <= 0.0 || total_size <= 0.0)
	{
		SetRanges(0.0, 1.0, 1.0, 1.0);
	}
	else
	{
		double scroll_max = std::max(1.0, total_size - view_size + 1.0);
		double page_step = std::max(1.0, view_size);
		SetRanges(0.0, scroll_max, 10, page_step);
	}
}

void Scrollbar::SetPosition(double pos)
{
	position = pos;
	if (pos >= scroll_max)
		position = scroll_max - 1.0;
	if (pos < scroll_min)
		position = scroll_min;

	if (UpdatePartPositions())
		Update();
}

void Scrollbar::OnMouseMove(const Point& pos)
{
	if (mouse_down_mode == mouse_down_thumb_drag)
	{
		double last_position = position;

		if (pos.x < -100.0 || pos.x > GetWidth() + 100.0 || pos.y < -100.0 || pos.y > GetHeight() + 100.0)
		{
			position = thumb_start_position;
		}
		else
		{
			double delta = vertical ? (pos.y - mouse_drag_start_pos.y) : (pos.x - mouse_drag_start_pos.x);
			double position_pixels = thumb_start_pixel_position + delta;

			double track_height = 0;
			if (vertical)
				track_height = rect_track_decrement.height + rect_track_increment.height;
			else
				track_height = rect_track_decrement.width + rect_track_increment.width;

			if (track_height != 0.0)
				position = scroll_min + position_pixels * (scroll_max - scroll_min) / track_height;
			else
				position = 0;

			if (position >= scroll_max)
				position = scroll_max - 1;
			if (position < scroll_min)
				position = scroll_min;

		}

		if (position != last_position)
		{
			InvokeScrollEvent(&FuncScrollThumbTrack);
			UpdatePartPositions();
		}
	}
	else if (mouse_down_mode == mouse_down_none)
	{
		part_button_decrement->SetStyleState(rect_button_decrement.contains(pos) ? "hover" : "");
		part_button_increment->SetStyleState(rect_button_increment.contains(pos) ? "hover" : "");
		part_thumb->SetStyleState(rect_thumb.contains(pos) ? "hover" : "");
	}
	Update();
}

bool Scrollbar::OnMouseDown(const Point& pos, InputKey key)
{
	mouse_drag_start_pos = pos;

	if (rect_button_decrement.contains(pos))
	{
		mouse_down_mode = mouse_down_button_decr;
		FuncScrollOnMouseDown = &FuncScrollLineDecrement;
		part_button_decrement->SetStyleState("down");

		double last_position = position;

		position -= line_step;
		last_step_size = -line_step;
		if (position >= scroll_max)
			position = scroll_max - 1.0;
		if (position < scroll_min)
			position = scroll_min;

		if (last_position != position)
			InvokeScrollEvent(&FuncScrollLineDecrement);
	}
	else if (rect_button_increment.contains(pos))
	{
		mouse_down_mode = mouse_down_button_incr;
		FuncScrollOnMouseDown = &FuncScrollLineIncrement;
		part_button_increment->SetStyleState("down");

		double last_position = position;

		position += line_step;
		last_step_size = line_step;
		if (position >= scroll_max)
			position = scroll_max - 1.0;
		if (position < scroll_min)
			position = scroll_min;

		if (last_position != position)
			InvokeScrollEvent(&FuncScrollLineIncrement);
	}
	else if (rect_thumb.contains(pos))
	{
		mouse_down_mode = mouse_down_thumb_drag;
		thumb_start_position = position;
		thumb_start_pixel_position = vertical ? (rect_thumb.y - rect_track_decrement.y) : (rect_thumb.x - rect_track_decrement.x);
		part_thumb->SetStyleState("down");
	}
	else if (rect_track_decrement.contains(pos))
	{
		mouse_down_mode = mouse_down_track_decr;
		FuncScrollOnMouseDown = &FuncScrollPageDecrement;

		double last_position = position;

		position -= page_step;
		last_step_size = -page_step;
		if (position >= scroll_max)
			position = scroll_max - 1.0;
		if (position < scroll_min)
			position = scroll_min;

		if (last_position != position)
			InvokeScrollEvent(&FuncScrollPageDecrement);
	}
	else if (rect_track_increment.contains(pos))
	{
		mouse_down_mode = mouse_down_track_incr;
		FuncScrollOnMouseDown = &FuncScrollPageIncrement;

		double last_position = position;

		position += page_step;
		last_step_size = page_step;
		if (position >= scroll_max)
			position = scroll_max - 1.0;
		if (position < scroll_min)
			position = scroll_min;

		if (last_position != position)
			InvokeScrollEvent(&FuncScrollPageIncrement);
	}

	mouse_down_timer->Start(100, false);

	UpdatePartPositions();

	Update();
	SetPointerCapture();
	return true;
}

bool Scrollbar::OnMouseUp(const Point& pos, InputKey key)
{
	if (mouse_down_mode == mouse_down_thumb_drag)
	{
		part_thumb->SetStyleState(rect_thumb.contains(pos) ? "hover" : "");
		if (FuncScrollThumbRelease)
			FuncScrollThumbRelease();
	}
	else if (mouse_down_mode == mouse_down_button_decr)
	{
		part_button_decrement->SetStyleState(rect_button_decrement.contains(pos) ? "hover" : "");
	}
	else if (mouse_down_mode == mouse_down_button_incr)
	{
		part_button_increment->SetStyleState(rect_button_increment.contains(pos) ? "hover" : "");
	}

	mouse_down_mode = mouse_down_none;
	mouse_down_timer->Stop();

	Update();
	ReleasePointerCapture();
	return true;
}

void Scrollbar::OnMouseLeave()
{
	if (part_button_decrement->GetStyleState() == "hover")
		part_button_decrement->SetStyleState("");
	if (part_button_increment->GetStyleState() == "hover")
		part_button_increment->SetStyleState("");
	if (part_thumb->GetStyleState() == "hover")
		part_thumb->SetStyleState("");
	Update();
}

void Scrollbar::OnGeometryChanged()
{
	UpdatePartPositions();
}

void Scrollbar::OnPaint(Canvas* canvas)
{
	if (part_thumb->IsStyled())
	{
		part_track->Paint(canvas, rect_track);
		part_button_decrement->Paint(canvas, rect_button_decrement);
		part_thumb->Paint(canvas, rect_thumb);
		if (auto image = part_thumb->GetStyleImage("grip-image"))
		{
			double w = part_thumb->GetStyleDouble("grip-image-width");
			double h = part_thumb->GetStyleDouble("grip-image-height");
			if (w == 0.0)
				w = (double)image->GetWidth();
			if (h == 0.0)
				h = (double)image->GetHeight();
			double x = rect_thumb.x + (rect_thumb.width - w) * 0.5;
			double y = rect_thumb.y + (rect_thumb.height - h) * 0.5;
			canvas->drawImage(image, Rect::xywh(x, y, w, h));
		}
		part_button_increment->Paint(canvas, rect_button_increment);
	}
	else
	{
		auto height = GetHeight();
		auto paint = rect_thumb.height < height;

		canvas->fillRect(Rect::shrink(Rect::xywh(0.0, 0.0, GetWidth(), height), 4.0, 0.0, 4.0, 0.0), GetStyleColor("track-color"));

		if (!paint) return;

		canvas->fillRect(Rect::shrink(rect_thumb, 4.0, 0.0, 4.0, 0.0), GetStyleColor("thumb-color"));
	}
}

// Calculates positions of all parts. Returns true if thumb position was changed compared to previously, false otherwise.
bool Scrollbar::UpdatePartPositions()
{
	if (parts_vertical != vertical || !part_button_increment)
	{
		parts_vertical = vertical;
		part_button_decrement = std::make_unique<PseudoWidget>(vertical ? "scrollbutton-up" : "scrollbutton-left");
		part_track = std::make_unique<PseudoWidget>(vertical ? "scrolltrack-vert" : "scrolltrack-horz");
		part_thumb = std::make_unique<PseudoWidget>(vertical ? "scrollthumb-vert" : "scrollthumb-horz");
		part_button_increment = std::make_unique<PseudoWidget>(vertical ? "scrollbutton-down" : "scrollbutton-right");
	}

	double decr_height = showbuttons ? 20.0 : 0.0;
	double incr_height = showbuttons ? 20.0 : 0.0;

	double total_height = vertical ? GetHeight() : GetWidth();
	double track_height = std::max(0.0, total_height - decr_height - incr_height);
	double thumb_height = CalculateThumbSize(track_height);

	double thumb_offset = decr_height + CalculateThumbPosition(thumb_height, track_height);

	Rect previous_rect_thumb = rect_thumb;

	rect_button_decrement = CreateRect(0.0, decr_height);
	rect_track_decrement = CreateRect(decr_height, thumb_offset);
	rect_track = CreateRect(0, total_height); // = CreateRect(decr_height, decr_height + track_height);
	rect_thumb = CreateRect(thumb_offset, thumb_offset + thumb_height);
	rect_track_increment = CreateRect(thumb_offset + thumb_height, decr_height + track_height);
	rect_button_increment = CreateRect(decr_height + track_height, decr_height + track_height + incr_height);

	return (previous_rect_thumb != rect_thumb);
}

double Scrollbar::CalculateThumbSize(double track_size)
{
	double minimum_thumb_size = 20.0;
	double range = scroll_max - scroll_min;
	double length = range + page_step - 1;
	double thumb_size = page_step * track_size / length;
	if (thumb_size < minimum_thumb_size)
		thumb_size = minimum_thumb_size;
	if (thumb_size > track_size)
		thumb_size = track_size;
	return thumb_size;
}

double Scrollbar::CalculateThumbPosition(double thumb_size, double track_size)
{
	double relative_pos = position - scroll_min;
	double range = scroll_max - scroll_min - 1;
	if (range != 0)
	{
		double available_area = std::max(0.0, track_size - thumb_size);
		return relative_pos * available_area / range;
	}
	else
	{
		return 0;
	}
}

Rect Scrollbar::CreateRect(double start, double end)
{
	if (vertical)
		return Rect(0.0, start, GetWidth(), end - start);
	else
		return Rect(start, 0.0, end - start, GetHeight());
}

void Scrollbar::OnTimerExpired()
{
	if (mouse_down_mode == mouse_down_thumb_drag)
		return;

	mouse_down_timer->Start(100, false);

	double last_position = position;
	position += last_step_size;
	if (position >= scroll_max)
		position = scroll_max - 1;

	if (position < scroll_min)
		position = scroll_min;

	if (position != last_position)
	{
		InvokeScrollEvent(FuncScrollOnMouseDown);

		if (UpdatePartPositions())
			Update();
	}
}

void Scrollbar::OnEnableChanged()
{
	Update();
}

void Scrollbar::InvokeScrollEvent(std::function<void()>* event_ptr)
{
	if (position == scroll_max - 1)
	{
		if (FuncScrollMax)
			FuncScrollMax();
	}

	if (position == scroll_min)
	{
		if (FuncScrollMin)
			FuncScrollMin();
	}

	if (FuncScroll)
		FuncScroll();

	if (event_ptr && *event_ptr)
		(*event_ptr)();
}
