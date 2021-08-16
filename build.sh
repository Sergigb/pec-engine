#!/bin/bash

# dumb bash script used to download and build dependencies + the project because I'm too lazy to learn cmake

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd $SCRIPT_DIR

if [[ ! -d tinyxml2 ]]; then
    git clone https://github.com/leethomason/tinyxml2.git
    cd tinyxml2
else
    cd tinyxml2
    git pull --ff-only
fi

make
cp libtinyxml2.a ../lib/

if [[ ! -d ../include/tinyxml2/ ]]; then
    mkdir ../include/tinyxml2/
fi
cp tinyxml2.h ../include/tinyxml2/

cd ..

if [[ ! -d bullet3 ]]; then
    git clone https://github.com/Sergigb/bullet3.git
    cd bullet3
else
    cd bullet3
    git pull --ff-only
fi

if [[ ! -d build ]]; then
    mkdir build
    cd build
    cmake -DUSE_DOUBLE_PRECISION=ON -DUSE_GRAPHICAL_BENCHMARK=OFF -DBULLET2_MULTITHREADING=ON -DBUILD_CPU_DEMOS=OFF -DBUILD_BULLET3=OFF ../
else
    cd build
fi

make
cp src/BulletCollision/libBulletCollision.a ../../lib/
cp src/BulletDynamics/libBulletDynamics.a ../../lib/
cp src/LinearMath/libLinearMath.a ../../lib/

cd ..
if [[ ! -d ../include/bullet/ ]]; then
    mkdir ../include/bullet/
fi
cp src/*.h ../include/bullet/
cp -r src/BulletCollision/ ../include/bullet/
cp -r src/BulletDynamics/ ../include/bullet/
cp -r src/LinearMath/ ../include/bullet/

cd ..

if [[ ! -d imgui ]]; then
    git clone https://github.com/ocornut/imgui.git
    cd imgui
else
    cd imgui
    git pull --ff-only
fi

if [[ ! -d ../thirdparty/imgui/ ]]; then
    mkdir ../thirdparty/imgui/
fi

cp *.cpp backends/imgui_impl_opengl3.cpp backends/imgui_impl_glfw.cpp ../thirdparty/imgui/
if [[ ! -d ../include/imgui/ ]]; then
    mkdir ../include/imgui/
fi
cp *.h backends/imgui_impl_glfw.h backends/imgui_impl_opengl3.h ../include/imgui

cd ..

if [[ ! -d assimp ]]; then
    git clone https://github.com/assimp/assimp.git
    cd assimp
else
    cd assimp
    git pull --ff-only
fi

if [[ ! -d build ]]; then
    mkdir build
    cd build
    cmake -DBUILD_SHARED_LIBS=OFF -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ZLIB=ON -DASSIMP_BUILD_ASSIMP_TOOLS=OFF \
          -DASSIMP_BUILD_SAMPLES=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_INSTALL=OFF -DASSIMP_BUILD_SAMPLES=OFF \
          -DASSIMP_BUILD_COLLADA_IMPORTER=ON -DASSIMP_BUILD_OBJ_IMPORTER=ON -DASSIMP_BUILD_GLTF_IMPORTER=ON \
          -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF ../

    # assimp cmake files are fucked, they don't use optimization flags in release so I append them myself
    FLAGS_LINE="$(grep -n "CXX_FLAGS" code/CMakeFiles/assimp.dir/flags.make)"
    IFS=':'
    read -ra arrFLAGS <<< "$FLAGS_LINE"
    arrFLAGS[1]="${arrFLAGS[1]} -O3"
    LINE="${arrFLAGS[0]}c\\"${arrFLAGS[1]}""
    sed -i $LINE code/CMakeFiles/assimp.dir/flags.make
else
    cd build
fi

make

cp contrib/zlib/libzlibstatic.a ../../lib/
cp lib/libassimp.a ../../lib

cd ..

if [[ ! -d ../include/assimp/ ]]; then
    mkdir ../include/assimp/
fi

cp -r include/assimp/* ../include/assimp
cp build/include/assimp/config.h ../include/assimp

cd ..


if [[ ! -d stb ]]; then
    git clone https://github.com/nothings/stb.git
    cd stb
else
    cd stb
    git pull --ff-only
fi

if [[ ! -d ../include/stb/ ]]; then
    mkdir ../include/stb/
fi

cp stb_image.h  stb_image_write.h ../include/stb

cd ../src/

make
