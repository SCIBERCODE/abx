/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   backward_svg;
    const int            backward_svgSize = 310;

    extern const char*   blind_svg;
    const int            blind_svgSize = 960;

    extern const char*   clear_svg;
    const int            clear_svgSize = 753;

    extern const char*   close_svg;
    const int            close_svgSize = 415;

    extern const char*   edit_svg;
    const int            edit_svgSize = 403;

    extern const char*   forward_svg;
    const int            forward_svgSize = 314;

    extern const char*   open_svg;
    const int            open_svgSize = 619;

    extern const char*   pause_svg;
    const int            pause_svgSize = 459;

    extern const char*   play_svg;
    const int            play_svgSize = 464;

    extern const char*   restart_svg;
    const int            restart_svgSize = 1527;

    extern const char*   rewind_svg;
    const int            rewind_svgSize = 452;

    extern const char*   settings_svg;
    const int            settings_svgSize = 898;

    extern const char*   stop_svg;
    const int            stop_svgSize = 372;

    extern const char*   warning_svg;
    const int            warning_svgSize = 543;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 14;

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
