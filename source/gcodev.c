/*   gcodev.c                                      F. Vernotte - 2019/06/20 */
/*   Computation of the Groslambert CoDeviation (GCoDev)                    */
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
#include <string.h>
#include <math.h>
#include "sigma_theta.h"

#define DATAMAX 16384
#define GRANMAX 67108864

void usage(void)
/* Help message */
    {
    printf("Usage: GCoDev SOURCE1 SOURCE2\n\n");
    printf("Computes the Groslambert CoDeviations (see arXiv:1904.05849) of 2 sequences of normalized frequency deviation measurements.\n\n");
    printf("The input files SOURCE1 and SOURCE2 contain a 2-column table with time values (dates) in the first column and normalized frequency deviation measurements in the second column.\n\n");
    printf("A 2-column table containing tau values (integration time) in the first column and Groslambert codeviation measurement in the second column is sent to the standard output.\n\n");
    printf("A redirection should be used for saving the results in a TARGET file: GCoDeV SOURCE1 SOURCE2 > TARGET.\n\n");
    printf("SigmaTheta %s %s - FEMTO-ST/OSU THETA/Universite de Franche-Comte/CNRS - FRANCE\n",st_version,st_date);
    }

int main(int argc, char *argv[])
/* Compute GCODEV serie from tau=tau0 (tau0=sampling step) to tau=N*tau0/2 (N=number of samples) by octave (log step : tau_n+1=2*tau_n) */
/* Input : file name of the normalized frequency deviation samples*/
/* Output : tau \t GCoDev(tau) */
/*          (for tau=tau0 to tau=N*tau0/2 by octave) */
    {
    int i,nbv,N,nto,tomax,err;
    long int dtmx;
    char source1[256], source2[256], gm[100];
    FILE *ofd;
    double v1,v2,smpt,rslt,tau[256],dev[256];

    if (argc<3)
        {
	usage();
	exit(-1);
        }
    else
	{
	strcpy(source1,*++argv);
	strcpy(source2,*++argv);
	}
    N=load_2yk(source1,source2);
    if (N==-1)
        printf("# File not found\n");
    else
        {
	if (N==-2) printf("# Different number of samples in %s and %s\n",source1,source2);
	else
		{
	        if (N<2)
		    {
        	    printf("# Unrecognized file\n\n");
		    if (N==1)
		      printf("# Use 1col2col command\n\n");
		    usage();
		    }
        	else
        	    {
	    	    err=init_flag();
	            if (err==-1) printf("# ~/.SigmaTheta.conf not found, default values selected\n");
	            if (err==-2) printf("# ~/.SigmaTheta.conf improper, default values selected\n");
	            if (err)
			{
	        	flag_log_inc=1;
	        	log_inc=(double)2;
			}
		    flag_variance=4;
		    nto=serie_dev(N, tau, dev);
        	    for(i=0;i<nto;++i)
        	        printf("%24.16e \t %24.16e\n",tau[i],dev[i]);
		    }
		}
        }
    }
    
