# jabdah
Modern cross-platform music player using Qt and BASS.

With Qt Creator:
1. Open Jabdah.pro with Qt Creator
2. Configure the project to use your own compiler
3. Set your application build to Release if it is not already set
4. Build your application

Without Qt Creator:
1. Open terminal to the folder of this program
2. qmake
3. make

For demonstration purposes BASS libraries from http://www.un4seen.com/ have been included. They will be removed in the future and users are required to download them manually. Same may happen to taglib too.

Please note that you'll need either bass.dll or libbass.so for Jabdah to run. Put this plugin to the same directory where your executable resided. In addition you'll need to create a directory named 'plugins' where you'll need to put:
bass_aac.dll/libbass_aac.so,
bass_ac3.dll/libbass_ac3.so,
bass_alac.dll/libbass_alac.so,
bass_ape.dll/libbass_ape.so,
bass_mpc.dll/libbass_mpc.so,
bass_spx.dll/libbass_spx.so,
bass_tta.dll/libbass_tta.so,
bassflac.dll/libbassflac.so,
bassmidi.dll/libbassmidi.so,
bassopus.dll/libbassopus.so,
basswv.dll/libbasswc.so
