Libraries that need to be put here:
* [Bullet Physics](https://github.com/bulletphysics/bullet3): ```libBulletCollision.a```, ```libBulletDynamics.a``` and ```libLinearMath.a``` (shared libs might work too).
* [Open Asset Import Library](https://github.com/assimp/assimp): ```libassimp.a```.
* [tinyxml2](https://github.com/leethomason/tinyxml2): ```libtinyxml2.a```.

Assimp has libz as a requirement, but it gives me problems to link it in Debian 11 (undefined symbols or whatever). I solved this by compiling assimp's static version of libz (set ASSIMP_BUILD_ZLIB to ON) and putting ```libzlibstatic.a``` here. It is found under ```contrib/zlib/libzlibstatic.a``` inside assimp's build directory.
