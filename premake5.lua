solution "opal"
  configurations { "Debug", "Release" }
  platforms {"x64"}

  configuration { "Debug" }
    targetdir "bin/debug"

  configuration { "Release" }
    targetdir "bin/release"

  if _ACTION == "clean" then
    os.rmdir("bin/debug", "bin/release")
  end

------------------------
------EXE
------------------------
  project "opal"
    language "C++"
    kind     "ConsoleApp"

    files  {"src/**.cpp", "include/**.h", "include/**.hpp" }
    links  { 
    "fmod64_vc",
    }

    libdirs { "lib/"}
    if(os.get() == "windows") then
      debugenvs "PATH=%PATH%;$(ProjectDir)"
      links {
        "wsock32",
        "Ws2_32"
      }
    elseif(os.get() == "linux") then
      buildoptions { "-std=c++11" }
      links { 
      "pthread", 
      }
    end
    if(_ACTION == "gmake") then
      buildoptions {"-std=c++11"}

      pchheader ( "include/CommonPrecompiled.h" )
    else
      pchheader ( "CommonPrecompiled.h" )
    end
    pchsource ( "src/CommonPrecompiled.cpp" )

    flags {
    "NoIncrementalLink", "StaticRuntime",
    "Unicode"}

    defines{
    }

    includedirs {
    "include",
    "include/**",    }
    flags { 
    "Unicode"}

    configuration { "Debug*" }
      defines { "_DEBUG", "DEBUG" }
      flags   { "Symbols" }
      libdirs { "lib/debug", 
      }

    configuration { "Release*" }
      defines { "NDEBUG" }
      libdirs { "lib/release",       }
