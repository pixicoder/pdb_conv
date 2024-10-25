**PalmOS PDB converter** 

Usage:
* source file(s) -> PDB file(s): pdb_conv [-w] [-bSIZE] [-tTYPE] srcfile1 [srcfile2] ...
* PDB file(s) -> original file(s): pdb_conv -r pdbfile1 [pdbfile2] ...

Options:
* -w - write PDB (optional; default);
* -r - read PDB;
* -bSIZE - set max block size (optional; default=54000);
* -tTYPE - set PDB file type (optional):
  * PSYX - default (PsyTexx2, SunVox);
  * MOD - PsyTexx1 MOD;
  * SNA - ArmZX;

Ways to copy PDB files to the PalmOS device:  
1) try to find pilot-link build for your system;  
2) use FileZ (or other PalmOS file browser) to copy PDB files from the SD card to the device's internal memory.

Please support my work:  
https://warmplace.ru/donate
