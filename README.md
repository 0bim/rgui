**OLD PROJECT** 
(couple years old, uploading just to archive/store it somewhere)

Only wrote this because I wanted things done a ceratain way at the time. 
Was used for a custom spotfiy overlay and external injectable mods to existing programs.

GUI framework mainly, had a small built in renderer for dx9 but wasn't used for long (I don't even remember if it has enough features to still be used), gif renderer using STB and both STB_TrueType and FreeType font rendering as well as the ability to rasterize SVGS using NanoSVG (I later reversed the svg parser from the software I was using it for and used that instead).
The Freetype renderer is not fully included here but I might add an example when I am done archiving other old projects)

The built in custom renderer is not that good because it was built to be used with a rendering API I reversed from the program I used this on.

dependencies:
[STB](https://github.com/nothings/stb) | [freetype](https://github.com/freetype/freetype) (optional, not implemented in this, might add the use example later) | [NanoSVG](https://github.com/memononen/nanosvg) | [NanoVG](https://github.com/memononen/nanovg) (was at one point used for drawing, but it is just an option, will work with any renderer rgui is basically just a "frontend/wrapper" for the most part)
 
