workspace("BirtualMachine")
configurations({"Debug", "Release"})

project("BirtualMachine")
kind("ConsoleApp")
language("C++")
targetdir("bin/%{cfg.buildcfg}")

files({"**.hpp", "**.cpp"})

cppdialect("C++20")
warnings("High")
flags("ShowCommandLine")

filter("configurations:Debug")
symbols("On")

filter("configurations:Release")
optimize("On")
