/**
 * Copyright (c) 2006-2015 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef LOVE_GRAPHICS_OPENGL_FONT_H
#define LOVE_GRAPHICS_OPENGL_FONT_H

// STD
#include <unordered_map>
#include <string>
#include <vector>

// LOVE
#include "common/config.h"
#include "common/Object.h"
#include "common/Matrix.h"
#include "common/Vector.h"

#include "font/Rasterizer.h"
#include "graphics/Texture.h"
#include "graphics/Volatile.h"
#include "VertexBuffer.h"

#include "OpenGL.h"

namespace love
{
namespace graphics
{
namespace opengl
{

class Font : public Object, public Volatile
{
public:

	enum AlignMode
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
		ALIGN_JUSTIFY,
		ALIGN_MAX_ENUM
	};

	struct GlyphVertex
	{
		float x, y;
		float s, t;
	};

	struct TextInfo
	{
		int width;
		int height;
	};

	// Used to determine when to change textures in the generated vertex array.
	struct DrawCommand
	{
		GLuint texture;
		int startvertex;
		int vertexcount;

		// used when sorting with std::sort.
		bool operator < (const DrawCommand &other) const
		{
			// Texture binds are expensive, so we should sort by that first.
			if (texture != other.texture)
				return texture < other.texture;
			else
				return startvertex < other.startvertex;
		}
	};

	Font(love::font::Rasterizer *r, const Texture::Filter &filter = Texture::getDefaultFilter());

	virtual ~Font();

	std::vector<DrawCommand> generateVertices(const std::string &text, std::vector<GlyphVertex> &vertices, float extra_spacing = 0.0f, Vector offset = Vector(), TextInfo *info = nullptr);
	std::vector<DrawCommand> generateVerticesFormatted(const std::string &text, float wrap, AlignMode align, std::vector<GlyphVertex> &vertices, TextInfo *info = nullptr);

	void drawVertices(const std::vector<DrawCommand> &drawcommands);

	/**
	 * Prints the text at the designated position with rotation and scaling.
	 *
	 * @param text A string.
	 * @param x The x-coordinate.
	 * @param y The y-coordinate.
	 * @param angle The amount of rotation.
	 * @param sx Scale along the x axis.
	 * @param sy Scale along the y axis.
	 * @param ox The origin offset along the x-axis.
	 * @param oy The origin offset along the y-axis.
	 * @param kx Shear along the x axis.
	 * @param ky Shear along the y axis.
	 **/
	void print(const std::string &text, float x, float y, float angle = 0.0f, float sx = 1.0f, float sy = 1.0f, float ox = 0.0f, float oy = 0.0f, float kx = 0.0f, float ky = 0.0f);

	void printf(const std::string &text, float x, float y, float wrap, AlignMode align, float angle = 0.0f, float sx = 1.0f, float sy = 1.0f, float ox = 0.0f, float oy = 0.0f, float kx = 0.0f, float ky = 0.0f);

	/**
	 * Returns the height of the font.
	 **/
	float getHeight() const;

	/**
	 * Returns the width of the passed string.
	 *
	 * @param str A string of text.
	 **/
	int getWidth(const std::string &str);

	/**
	 * Returns the width of the passed character.
	 *
	 * @param character A character.
	 **/
	int getWidth(char character);

	/**
	 * Returns the maximal width of a wrapped string
	 * and optionally the number of lines
	 *
	 * @param text The input text
	 * @param wrap The number of pixels to wrap at
	 * @param max_width Optional output of the maximum width
	 * @param wrapped_lines Optional output indicating which lines were
	 *        auto-wrapped. Indices correspond to indices of the returned value.
	 * Returns a vector with the lines.
	 **/
	void getWrap(const std::string &text, float wrap, std::vector<std::string> &lines, std::vector<int> *line_widths = 0, std::vector<bool> *wrapped_lines = 0);

	/**
	 * Sets the line height (which should be a number to multiply the font size by,
	 * example: line height = 1.2 and size = 12 means that rendered line height = 12*1.2)
	 * @param height The new line height.
	 **/
	void setLineHeight(float height);

	/**
	 * Returns the line height.
	 **/
	float getLineHeight() const;

	void setFilter(const Texture::Filter &f);
	const Texture::Filter &getFilter();

	// Implements Volatile.
	bool loadVolatile();
	void unloadVolatile();

	// Extra font metrics
	int getAscent() const;
	int getDescent() const;
	float getBaseline() const;

	bool hasGlyph(uint32 glyph) const;
	bool hasGlyphs(const std::string &text) const;

	uint32 getTextureCacheID() const;

	static bool getConstant(const char *in, AlignMode &out);
	static bool getConstant(AlignMode in, const char  *&out);

	static int fontCount;

private:

	enum FontType
	{
		FONT_TRUETYPE,
		FONT_IMAGE,
		FONT_UNKNOWN
	};

	struct Glyph
	{
		GLuint texture;
		int spacing;
		GlyphVertex vertices[4];
	};

	struct TextureSize
	{
		int width;
		int height;
	};

	TextureSize getNextTextureSize() const;
	void createTexture();
	love::font::GlyphData *getRasterizerGlyphData(uint32 glyph);
	const Glyph &addGlyph(uint32 glyph);
	const Glyph &findGlyph(uint32 glyph);
	void printv(const Matrix &t, const std::vector<DrawCommand> &drawcommands, const std::vector<GlyphVertex> &vertices);

	StrongRef<love::font::Rasterizer> rasterizer;

	int height;
	float lineHeight;

	int textureWidth;
	int textureHeight;

	// vector of packed textures
	std::vector<GLuint> textures;

	// maps glyphs to glyph texture information
	std::unordered_map<uint32, Glyph> glyphs;

	FontType type;
	Texture::Filter filter;

	int textureX, textureY;
	int rowHeight;

	bool useSpacesAsTab;

	// Index buffer used for drawing quads with GL_TRIANGLES.
	VertexIndex indexBuffer;

	// ID which is incremented when the texture cache is invalidated.
	uint32 textureCacheID;

	size_t textureMemorySize;

	static const int TEXTURE_PADDING = 1;

	// This will be used if the Rasterizer doesn't have a tab character itself.
	static const int SPACES_PER_TAB = 4;

	static StringMap<AlignMode, ALIGN_MAX_ENUM>::Entry alignModeEntries[];
	static StringMap<AlignMode, ALIGN_MAX_ENUM> alignModes;

}; // Font

} // opengl
} // graphics
} // love

#endif // LOVE_GRAPHICS_OPENGL_FONT_H
