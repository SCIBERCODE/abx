/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   upcdl_ttf;
    const int            upcdl_ttfSize = 70912;

    extern const char*   backward_svg;
    const int            backward_svgSize = 245;

    extern const char*   blind_svg;
    const int            blind_svgSize = 880;

    extern const char*   clear_svg;
    const int            clear_svgSize = 679;

    extern const char*   close_svg;
    const int            close_svgSize = 352;

    extern const char*   edit_svg;
    const int            edit_svgSize = 339;

    extern const char*   forward_svg;
    const int            forward_svgSize = 247;

    extern const char*   open_svg;
    const int            open_svgSize = 542;

    extern const char*   pause_svg;
    const int            pause_svgSize = 146;

    extern const char*   play_svg;
    const int            play_svgSize = 127;

    extern const char*   restart_svg;
    const int            restart_svgSize = 679;

    extern const char*   rewind_svg;
    const int            rewind_svgSize = 144;

    extern const char*   settings_svg;
    const int            settings_svgSize = 786;

    extern const char*   stop_svg;
    const int            stop_svgSize = 126;

    extern const char*   warning_svg;
    const int            warning_svgSize = 425;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 15;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
