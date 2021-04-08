This is a small readme for generating the Nanopb files and Doxygen documentation:

Prerequisites:
- LaTex installed
    - sudo apt-get install texlive texlive-latex-extra
- Java installed (for UML charts)
    - sudo apt-get install default-jre
    - PlantUML
      - sudo apt-get install graphviz


Nanopb:

- Download the latest binary Nanopb version release from here: http://koti.kapsi.fi/~jpa/nanopb/download/
- Unpack the release e.g. "tar -xzf nanopb-0.3.3-linux-x86.tar.gz"
- On Ubuntu 14.04 64-bit install the 32-bit support files: "sudo apt-get install lib32z1 lib32ncurses5 lib32bz2-1.0 libstdc++6:i386"
- Export the path to Nanopb e.g. in the .bashrc file: "export NANOPB=~/nanopb-0.3.3-linux-x86"
- Open a new shell (to load the .bashrc changes)
- Run "make" in the current directory
- The Nanopb files should now be generated

Doxygen (Ubuntu 14.04 64-bit):

- Download the latest Doxygen release from here: https://www.stack.nl/~dimitri/doxygen/download.html
- Unpack the release e.g. "tar -xzf doxygen-1.8.10.linux.bin.tar.gz"
- Export the path to Doxygen e.g. in the .bashrc file: "export DOXYGEN=~/doxygen-1.8.10/bin/doxygen"
- Open a new shell (to load the .bashrc changes)
- run "make doc" in the current directory
- The Doxygen PDF should now be generated

Doxygen (Ubuntu 14.04 32-bit):

- Download the latest Doxygen source distribution from here: https://www.stack.nl/~dimitri/doxygen/download.html
- Unpack the release e.g. "tar -xzf doxygen-1.8.10.src.tar.gz"
- Install required packages to build Doxygen
    - sudo apt-get install cmake
    - sudo apt-get install bison
    - sudo apt-get install flex
- Follow the instructions in BUILD.txt to build Doxygen
- Export the path to Doxygen e.g. in the .bashrc file: "export DOXYGEN=~/doxygen-1.8.10/bin/build/doxygen"
- Open a new shell or source .bashrc (to load the .bashrc changes)
- Install required packages to run Doxygen
    - sudo apt-get install default-jre
    - sudo apt-get install texlive
- run "make doc" in the current directory
- The Doxygen PDF should now be generated
