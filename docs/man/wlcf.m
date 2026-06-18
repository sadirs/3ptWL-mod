't" t
.TH wlcf 1 "May 2026" UNIX "LSST/S3 PROJECT"
.na
.nh

.SH NAME
wlcf \- Weak Lensing Correlation Functions computation code

.SH SYNOPSIS
\fBwlcf\fR [ \fIparameter_file_name\fR ] [ \fIoptions\fR ]
.sp

.SH DESCRIPTION
\fBwlcf\fR computes weak lensing correlation functions using perturbation theory,
effective field theory, and halo-model inspired methods.

The code is designed to compute multipoles of the three-point correlation
function (3PCF) of the weak lensing convergence field in a plane-wave expansion.

.SH OPTIONS
All command-line options have the structure
.sp
$ \fIoption_name\fR=<option_value>
.sp
except the option \fB--help\fR. No spaces should be used between
\fIoption_name\fR, '=' and \fIoption_value\fR in command-line mode.
.sp

.IP "\fB--help\fR" 12
Prints the list of available parameters and their default values:
.sp
$ wlcf --help
.sp

Some options may have aliases. In the help output, aliases are indicated with
the tag [a: alias]. For example:
.sp
option_name=<option_value>     Description     [a: opt]
.sp

.IP "\fBparamfile\fR" 12
Name of the file containing input parameter values. Parameters in this file
overwrite default values.
.sp
$ wlcf paramfile=parameters_input_file
.sp

Parameter files can be created manually. Comment lines start with "#" or "%".
In a parameter file, use the format:
.sp
option_name = option_value
.sp

The order of parameters does not matter. Running the code with default values:
.sp
$ wlcf
.sp
creates a file with the parameters used in the run, for example:
.sp
a file ending in -usedvalues
.sp
or a similarly named file inside the output directory. This file can be used
as a template for custom parameter files.

.SH COSMOLOGICAL PARAMETERS

.IP "\fBz\fR" 12
Redshift used in the computation.

.IP "\fBh\fR" 12
Dimensionless Hubble parameter.

.IP "\fBsigma8\fR" 12
[a: s8] Normalization of the matter power spectrum.

.IP "\fBOmb\fR" 12
Baryon density parameter.

.IP "\fBOmc\fR" 12
Cold dark matter density parameter.

.IP "\fBOmnu\fR" 12
Massive neutrino density parameter.

.IP "\fBns\fR" 12
Spectral index of the linear power spectrum.

.IP "\fBw\fR" 12
Dark energy equation of state parameter.

.SH INPUT PARAMETERS

.IP "\fBfnamePS\fR" 12
[a: ps] Linear matter power spectrum file.

.IP "\fBWg\fR" 12
Weak lensing kernel option. If Wg=0, the code uses a Dirac delta at the
redshift bin. If Wg=1, the kernel is read from file.

.IP "\fBfWgchi\fR" 12
[a: fwgchi] File containing the weak lensing kernel Wg(chi).

.SH MODEL AND NUMERICAL PARAMETERS

.IP "\fBtree_level\fR" 12
Selects the theoretical model used for the bispectrum calculation:
.sp
1 = Standard Perturbation Theory (SPT)
.br
2 = P2 approximation
.br
3 = Effective Field Theory (EFT)
.br
4 or larger = Takahashi/Halo-model inspired bispectrum

.IP "\fBzbin\fR" 12
Redshift bin used in the projection.

.IP "\fBmMax\fR" 12
Maximum multipole order to compute.

.IP "\fBchiQuadSteps\fR" 12
[a: chiqst] Number of integration steps in comoving distance.

.IP "\fBGLpoints\fR" 12
[a: gl] Number of Gauss-Legendre integration points.

.IP "\fBNell\fR" 12
Number of angular multipole values.

.IP "\fBellmin\fR" 12
Minimum multipole value.

.IP "\fBellmax\fR" 12
Maximum multipole value.

.SH OUTPUT PARAMETERS

.IP "\fBrootDir\fR" 12
[a: root] Output directory where files will be written. If the directory does
not exist, it will be created.

.IP "\fBprefix\fR" 12
[a: pre] Prefix used for output file names.

.IP "\fBwritevectors\fR" 12
If true, intermediate vectors are written to output files.

.SH MISCELLANEOUS PARAMETERS

.IP "\fBverbose\fR" 12
[a: verb] Controls the amount of information printed to standard output.

.IP "\fBverbose_log\fR" 12
[a: verblog] Controls the amount of information written to the log file.

.IP "\fBnumberThreads\fR" 12
[a: nthreads] Number of OpenMP threads used during execution. OpenMP must be
enabled at compilation time.

.IP "\fBoptions\fR" 12
[a: opt] Additional code behavior options.

.SH EXAMPLES

Compile the executable:
.sp
$ make clean; make
.sp

Compile the executable and Python wrapper:
.sp
$ make clean; make all
.sp

Run the default test:
.sp
$ cd tests
.br
$ ../wlcf
.sp

Run with a custom linear power spectrum:
.sp
$ ../wlcf ps=./input/linear_pk_Takahashi_z0.txt verb=2
.sp

Run the EFT branch:
.sp
$ ../wlcf tree_level=3 ps=./input/linear_pk_Takahashi_z0.txt
.sp

Run the Takahashi/Halo-model branch:
.sp
$ ../wlcf tree_level=4 ps=./input/linear_pk_Takahashi_z0.txt
.sp

.SH PYTHON INTERFACE

The Python wrapper can be used as:
.sp
$ python
.sp
>>> from wlcfpy import wlcf
.br
>>> w = wlcf()
.br
>>> w.set(numberThreads=8, tree_level=4)
.br
>>> w.Run()
.sp

.SH OUTPUT FILES

The code writes output files using the selected prefix. With the default
prefix, typical output files include:
.sp
run1_theta_array.txt
.br
run1_kArray.txt
.br
run1_ellArray.txt
.br
run1_Bmells_*.txt
.br
run1_Bnk_*.txt
.br
run1_zetam*.txt
.sp

The files zetam*.txt contain the multipoles of the 3PCF,
zeta_m(theta1, theta2).

.SH SEE ALSO
wlcfpy documentation in the Sphinx manual.

.SH COPYRIGHT
Copyright (C) 2026--
.br
Alejandro Aviles et al.
.br
