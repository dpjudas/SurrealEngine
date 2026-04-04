
#include "border_image_renderer.h"
#include "surrealwidgets/core/image.h"
#include "surrealwidgets/core/canvas.h"
#include <cmath>

void BorderImageRenderer::render(Canvas* canvas, const BorderGeometry& geometry, const BorderImage& style)
{
	BorderImageRenderer renderer(canvas, geometry, style);
	renderer.render();
}

BorderImageRenderer::BorderImageRenderer(Canvas* canvas, const BorderGeometry& geometry, const BorderImage& style) : canvas(canvas), geometry(geometry), style(style)
{
}

void BorderImageRenderer::render()
{
	std::shared_ptr<Image> image = style.source;
	if (image)
	{
		int slice_left = get_left_slice_value(image->GetWidth());
		int slice_right = get_right_slice_value(image->GetWidth());
		int slice_top = get_top_slice_value(image->GetHeight());
		int slice_bottom = get_bottom_slice_value(image->GetHeight());
		bool fill_center = style.slice.center;

		Box border_image_area = get_border_image_area();

		float grid_left = get_left_grid(border_image_area.get_width(), (float)slice_left);
		float grid_right = get_right_grid(border_image_area.get_width(), (float)slice_right);
		float grid_top = get_top_grid(border_image_area.get_height(), (float)slice_top);
		float grid_bottom = get_bottom_grid(border_image_area.get_height(), (float)slice_bottom);

		float x[4] = { border_image_area.left, border_image_area.left + grid_left, border_image_area.right - grid_right, border_image_area.right };
		float y[4] = { border_image_area.top, border_image_area.top + grid_top, border_image_area.bottom - grid_bottom, border_image_area.bottom };
		int sx[4] = { 0, slice_left, image->GetWidth() - slice_right, image->GetWidth() };
		int sy[4] = { 0, slice_top, image->GetHeight() - slice_bottom, image->GetHeight() };

		BorderImageRepeat repeat_x = style.repeat.x;
		BorderImageRepeat repeat_y = style.repeat.y;

		for (int yy = 0; yy < 3; yy++)
		{
			for (int xx = 0; xx < 3; xx++)
			{
				if ((xx != 1 && yy != 1) || fill_center)
					draw_area(image, x[xx], y[yy], x[xx + 1] - x[xx], y[yy + 1] - y[yy], sx[xx], sy[yy], sx[xx + 1] - sx[xx], sy[yy + 1] - sy[yy], repeat_x, repeat_y);
			}
		}
	}
}

BorderImageRenderer::TileRepeatInfo BorderImageRenderer::repeat_info(float x, float w, int sw, BorderImageRepeat repeat)
{
	TileRepeatInfo info;

	if (sw <= 0)
		return info;

	if (repeat == BorderImageRepeat::repeat)
	{
		info.count = (int)std::ceil(w / sw);
		info.start = x + (w - info.count * sw) * 0.5f;
		info.width = (float)sw;
	}
	else if (repeat == BorderImageRepeat::stretch)
	{
		info.start = x;
		info.width = w;
		info.count = 1;
	}
	else if (repeat == BorderImageRepeat::round)
	{
		info.start = x;
		info.count = std::max((int)std::round(w / sw), 1);
		info.width = sw * w / (sw * info.count);
	}
	else if (repeat == BorderImageRepeat::space)
	{
		info.start = x;
		info.width = (float)sw;
		info.count = (int)std::floor(w / sw);
		if (info.count > 1)
			info.space = w / (info.count - 1) - sw;
		else if (info.count == 1)
			info.start = x + (w - info.count * sw) * 0.5f;
	}

	return info;
}

void BorderImageRenderer::draw_area(const std::shared_ptr<Image>& image, float x, float y, float w, float h, int sx, int sy, int sw, int sh, BorderImageRepeat repeat_x, BorderImageRepeat repeat_y)
{
	TileRepeatInfo tile_x = repeat_info(x, w, sw, repeat_x);
	TileRepeatInfo tile_y = repeat_info(y, h, sh, repeat_y);

	Box clip = Box::xywh(x, y, w, h);
	Box src = Box::xywh((float)sx, (float)sy, (float)sw, (float)sh);

	for (int yy = 0; yy < tile_y.count; yy++)
	{
		float top = tile_y.start + (tile_y.width + tile_y.space) * yy;
		float bottom = tile_y.start + (tile_y.width + tile_y.space) * (yy + 1) - tile_y.space;

		for (int xx = 0; xx < tile_x.count; xx++)
		{
			float left = tile_x.start + (tile_x.width + tile_x.space) * xx;
			float right = tile_x.start + (tile_x.width + tile_x.space) * (xx + 1) - tile_x.space;

			Box dest(left, top, right, bottom);

			Box dest_clipped = dest;
			dest_clipped.clip(clip);

			float tleft = (dest_clipped.left - dest.left) / dest.get_width();
			float tright = (dest_clipped.right - dest.left) / dest.get_width();
			float ttop = (dest_clipped.top - dest.top) / dest.get_height();
			float tbottom = (dest_clipped.bottom - dest.top) / dest.get_height();

			Box src_clipped(mix(src.left, src.right, tleft), mix(src.top, src.bottom, ttop), mix(src.left, src.right, tright), mix(src.top, src.bottom, tbottom));

			canvas->drawImage(
				image,
				Rect::ltrb(src_clipped.left, src_clipped.top, src_clipped.right, src_clipped.bottom),
				Rect::ltrb(dest_clipped.left, dest_clipped.top, dest_clipped.right, dest_clipped.bottom));
		}
	}
}

BorderImageRenderer::Box BorderImageRenderer::get_border_image_area() const
{
	Box box = geometry.box;

	BorderImageValue outset_left = style.outset.left;
	BorderImageValue outset_right = style.outset.right;
	BorderImageValue outset_top = style.outset.top;
	BorderImageValue outset_bottom = style.outset.bottom;

	if (outset_left.is_length() || outset_left.is_number())
		box.left -= outset_left.number();

	if (outset_right.is_length() || outset_right.is_number())
		box.right += outset_right.number();

	if (outset_top.is_length() || outset_top.is_number())
		box.top -= outset_top.number();

	if (outset_bottom.is_length() || outset_bottom.is_number())
		box.bottom += outset_bottom.number();

	return box;
}

float BorderImageRenderer::get_left_grid(float image_area_width, float auto_width) const
{
	BorderImageValue border_image_width = style.width.left;

	if (border_image_width.is_percentage())
		return border_image_width.number() * image_area_width / 100.0f;
	else if (border_image_width.is_length())
		return border_image_width.number();
	else if (border_image_width.is_number())
		return border_image_width.number() * (float)geometry.border.left;
	else
		return auto_width;
}

float BorderImageRenderer::get_right_grid(float image_area_width, float auto_width) const
{
	BorderImageValue border_image_width = style.width.right;

	if (border_image_width.is_percentage())
		return border_image_width.number() * image_area_width / 100.0f;
	else if (border_image_width.is_length())
		return border_image_width.number();
	else if (border_image_width.is_number())
		return border_image_width.number() * (float)geometry.border.right;
	else
		return auto_width;
}

float BorderImageRenderer::get_top_grid(float image_area_height, float auto_height) const
{
	BorderImageValue border_image_width = style.width.top;

	if (border_image_width.is_percentage())
		return border_image_width.number() * image_area_height / 100.0f;
	else if (border_image_width.is_length())
		return border_image_width.number();
	else if (border_image_width.is_number())
		return border_image_width.number() * (float)geometry.border.top;
	else
		return auto_height;
}

float BorderImageRenderer::get_bottom_grid(float image_area_height, float auto_height) const
{
	BorderImageValue border_image_width = style.width.bottom;

	if (border_image_width.is_percentage())
		return border_image_width.number() * image_area_height / 100.0f;
	else if (border_image_width.is_length())
		return border_image_width.number();
	else if (border_image_width.is_number())
		return border_image_width.number() * (float)geometry.border.bottom;
	else
		return auto_height;
}

int BorderImageRenderer::get_left_slice_value(int image_width) const
{
	BorderImageValue border_image_slice = style.slice.left;

	int v = 0;
	if (border_image_slice.is_percentage())
		v = (int)std::round(border_image_slice.number() * image_width / 100.0f);
	else
		v = (int)std::round(border_image_slice.number());
	return std::max(0, std::min(image_width, v));
}

int BorderImageRenderer::get_right_slice_value(int image_width) const
{
	BorderImageValue border_image_slice = style.slice.right;

	int v = 0;
	if (border_image_slice.is_percentage())
		v = (int)std::round(border_image_slice.number() * image_width / 100.0f);
	else
		v = (int)std::round(border_image_slice.number());
	return std::max(0, std::min(image_width, v));
}

int BorderImageRenderer::get_top_slice_value(int image_height) const
{
	BorderImageValue border_image_slice = style.slice.top;

	int v = 0;
	if (border_image_slice.is_percentage())
		v = (int)std::round(border_image_slice.number() * image_height / 100.0f);
	else
		v = (int)std::round(border_image_slice.number());
	return std::max(0, std::min(image_height, v));
}

int BorderImageRenderer::get_bottom_slice_value(int image_height) const
{
	BorderImageValue border_image_slice = style.slice.bottom;

	int v = 0;
	if (border_image_slice.is_percentage())
		v = (int)std::round(border_image_slice.number() * image_height / 100.0f);
	else
		v = (int)std::round(border_image_slice.number());
	return std::max(0, std::min(image_height, v));
}
