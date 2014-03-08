RpmDownloader
=============

RpmDownloader is a small application which can mirror a subset of RPMs from a specified repository. But it is more than a simple mirror tool it helps you finding the RPM, can resolve dependencies (only for the selected repository and only yum repositories), informs you if a newer version is available and then downloads only the updated/missing packages.

Why this tool?

I've developed this tool for two reasons: I sometimes build RPMs for openSuSE (especially zoneminder) and for that I need a subset of RPMs from the packman repository, which contents changes very frequently. I want to get informed if a newer version is available and only download the changed RPMs. Second reason was that I wanted to play a bit around with Qt :)

Who might find this tool interesting? Maybe you want to do something similar then you might want to try this application. Another possibility is that you want to download packages to make them available offline, this tool can then resolve the dependencies (only for the selected repository) and adds all packages which are needed from the repository to install the package.

