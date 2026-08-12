#pragma once

class UModel;
class BspSurface;

class Shadowmap
{
public:
	Shadowmap();

	void Load(UModel* model, int lightMap, int lightindex);
	void Clear(UModel* model, int lightMap);

	int Width() const { return width; }
	int Height() const { return height; }
	const float* Pixels() const { return pixels.data(); }

private:
	int width = 0;
	int height = 0;
	Array<float> pixels;
	Array<float> tempbuf;
	Array<float> blurTable;
};
