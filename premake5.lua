workspace("BirtualMachine")
configurations({"Debug", "Release"})

project("BirtualMachine")
kind("ConsoleApp")
language("C++")
targetdir("bin/%{cfg.buildcfg}")

files({"**.hpp", "**.cpp"})

cppdialect("C++20")

filter("configurations:Debug")
symbols("On")

filter("configurations:Release")
optimize("On")
