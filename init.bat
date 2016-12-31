git submodule init
git submodule update
curl -o curl-7.50.1.tar.gz https://curl.haxx.se/download/curl-7.50.1.tar.gz 
tar xvzf curl-7.50.1.tar.gz
(
  cd curl-7.50.1\winbuild
  nmake /f Makefile.vc mode=dll ENABLE_WINSSL=yes VC=14 GEN_PDB=yes DEBUG=yes MACHINE=x86
  nmake /f Makefile.vc mode=dll ENABLE_WINSSL=yes VC=14 GEN_PDB=yes DEBUG=no MACHINE=x86
)

cd modules/cryptopp
cmake -G "Visual Studio 14 2015"
msbuild cryptopp.sln /t:cryptlib /p:Configuration=debug
