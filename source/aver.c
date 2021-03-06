/*   aver.c                                    F. Vernotte - 2018/10/19     */
/*   Average rounded to integer of 2nd columns of file 1 and file 2         */
/*				   Modified FV 30/07/2019 (adding options)  */
/*                                                                          */
/*                                                   - SIGMA-THETA Project  */
/*                                                                          */
/* Copyright or © or Copr. Université de Franche-Comté, Besançon, France    */
/* Contributor: François Vernotte, UTINAM/OSU THETA (2012/07/17)            */
/* Contact: francois.vernotte@obs-besancon.fr                               */
/*                                                                          */
/* This software, SigmaTheta, is a collection of computer programs for      */
/* time and frequency metrology.                                            */
/*                                                                          */
/* This software is governed by the CeCILL license under French law and     */
/* abiding by the rules of distribution of free software.  You can  use,    */
/* modify and/ or redistribute the software under the terms of the CeCILL   */
/* license as circulated by CEA, CNRS and INRIA at the following URL        */
/* "http://www.cecill.info".                                                */
/*                                                                          */
/* As a counterpart to the access to the source code and  rights to copy,   */
/* modify and redistribute granted by the license, users are provided only  */
/* with a limited warranty  and the software's author,  the holder of the   */
/* economic rights,  and the successive licensors  have only  limited       */
/* liability.                                                               */
/*                                                                          */
/* In this respect, the user's attention is drawn to the risks associated   */
/* with loading,  using,  modifying and/or developing or reproducing the    */
/* software by the user in light of its specific status of free software,   */
/* that may mean  that it is complicated to manipulate,  and  that  also    */
/* therefore means  that it is reserved for developers  and  experienced    */
/* professionals having in-depth computer knowledge. Users are therefore    */
/* encouraged to load and test the software's suitability as regards their  */
/* requirements in conditions enabling the security of their systems and/or */
/* data to be ensured and,  more generally, to use and operate it in the    */
/* same conditions as regards security.                                     */
/*                                                                          */
/* The fact that you are presently reading this means that you have had     */
/* knowledge of the CeCILL license and that you accept its terms.           */
/*                                                                          */
/*                                                                          */
                                    
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "sigma_theta.h"

#define DATAMAX 16384
#define GRANMAX 67108864

void usage(void)
/* Help message */
    {
    printf("Usage: AVer [-s|q|c'val'] FILE1 FILE2 FILE3\n\n");
    printf("Computes the average of the 2nd columns of FILE1, FILE2 and FILE3: (y1+y2+y3)/3.\n\n");
    printf("The input files FILE1, FILE2 and FILE3 contain a N line / 2 column table, the 1st columns are assumed identical.\n\n");
    printf("A 2-column table containing the 1st column of FILE1 as the first column of the output table and the average of the 2nd columns of FILE1, FILE2, FILE3 as the the second column of the output table is sent to the standard output.\n\n");
    printf("If the option '-s' is selected, the sum is computed without dividing by 3. \n");
    printf("If the option '-q' is selected, the sum is divided by the square root of 3 (suitable to compute the measurement noise of a three-cornered system thanks to the closure relationship). \n");
    printf("If the option '-c' immediately followed by a value 'val' is selected, the sum is divided by the coefficient 'val'.\n\n");
    printf("A redirection should be used for saving the results in a TARGET file: AVer FILE1 FILE2 > TARGET.\n\n");
    printf("SigmaTheta %s %s - FEMTO-ST/OSU THETA/Universite de Franche-Comte/CNRS - FRANCE\n",st_version,st_date);
    }

int main(int argc, char *argv[])
    {
    int rep, rip, indeq, err;
    int i, nbv, N, N2, N3;
    double knorm;
    char source1[256], source2[256], source3[256], gm[100];

    knorm=(double)3;
    while ((rep = getopt (argc, argv, "sqc:?")) != -1)
	switch(rep)
		{
		case 's':
			knorm = (double)1;
			break;
		case 'q':
			knorm = sqrt((double)3);
			break;
		case 'c':
			knorm=0;
			rip=sscanf(optarg,"%lf",&knorm);
			if ((rip!=1)||(!knorm))
				{
				printf("# Invalid coefficient after option '-c'\n");
				usage();
				exit(-1);
				}
			break;
		case '?':
			printf("# Unknown option '-%c'\n",optopt);
			usage();
		default:
			exit(-1);
		}
    if (optind>2)
	{
	printf("Incompatible options\n");
	usage();
	exit(-1);
	}
    if (argc-optind<3)
	{
	printf("# Missing arguments\n");
	usage();
	exit(-1);
	}
    if (argc-optind>3)
	{
	printf("# Too many arguments\n");
	usage();
	exit(-1);
	}
    strcpy(source1,argv[optind]);
    strcpy(source2,argv[optind+1]);
    strcpy(source3,argv[optind+2]);

    err=0;
    printf("# Coefficient: %f\n",knorm);
    N=load_3yk(source1,source2,source3);
    if (N<1)
	{
	switch(N)
		{
	        case 0:
			printf("# Empty file\n");
			break;
		case -1:
			printf("# File not found\n");
			break;
		case -2:
			printf("# Different file lengths\n");
		}
	exit(N);
	}
    for (i=0;i<N;++i)
	printf("%24.16e \t %24.16e\n",T[i],(Y12[i]+Y23[i]+Y31[i])/knorm);
    exit(err);		
    }
    
