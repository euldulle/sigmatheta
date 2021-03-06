/*   uncertainties.c                              F. Vernotte - 2010/10/24  */
/*   Computation of the uncertainties of a adev or mdev measurement serie   */
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

#define db(x) ((double)(x))
#define sisig(x) ( (x) == 0 ) ? (db(0)) : (  ( (x) > 0 ) ? (db(1)) : (db(-1))  )

void usage(void)
/* Help message */
    {
    printf("Usage: uncertainties [-option] SOURCE TARGET\n\n");
    printf("Computes the 95 %% confidence intervals of a sequence of (modified) Allan Deviations, the asymptotes and plot a graph.\n\n");
    printf("The input file SOURCE contains a 2-column table with tau values (integration time) in the first column and (modified) Allan deviation measurement in the second column.\n\n");
    printf("The first tau value is assumed to be equal to the sampling step.\n");
    printf("The last tau value is assumed to be equal to the half of the whole time sequence duration.\n\n");
    printf("A 7-column table is sent to the standard output with:\n");
    printf("\t1st column: tau values\n");
    printf("\t2nd column: (modified) Allan deviation estimate\n");
    printf("\t3rd column: unbiased estimate\n");
    printf("\t4th column: 2.5 %% bound\n");
    printf("\t5th column: 16 %% bound\n");
    printf("\t6th column: 84 %% bound\n");
    printf("\t7th column: 97.5 %% bound.\n\n");
    printf("The file TARGET.gnu is generated for invoking gnuplot. The configuration file \".SigmaTheta.conf\" is taken into account.\n");
    printf("The file TARGET.ps is the postscript file of the gnuplot graph.\n\n");
    printf("If the option '-m' is selected, the variance is assumed to be the modified Allan variance.\n");
    printf("If the option '-c' is selected, the variance is assumed to be the classical Allan variance.\n");
    printf("Otherwise, the variance is assumed to be the one selected in the configuration file \".SigmaTheta.conf\".\n\n"); 
    printf("SigmaTheta %s %s - FEMTO-ST/OSU THETA/Universite de Franche-Comte/CNRS - FRANCE\n",st_version,st_date);
    }

int main(int argc, char *argv[])
/* Compute the coefficients of the tau^-3/2, tau^-1, tau^-1/2, tau^0, tau^1/2 and tau of a {tau,adev} serie */
/* Input : tau and adev value file   */
/* Output: 6 asymptotes coefficients */
    {
    int i,j,N,err,alpha[32];
    double tsas,asympt,tau[32], adev[32], avar[32], edf[32], bmin[32], bmax[32],bi1s[32],bx1s[32],adc[32],w[32];
    char pre_flag_v, source[256], outfile[256], command[32];
    struct conf_int rayl;
    FILE *ofd;

    pre_flag_v=0;
    if ((argc<3)||(argc>4))
        {
        usage();
	exit(-1);
	}
    else
	if (argc==3)
        	{
		strcpy(source,*++argv);
		strcpy(outfile,*++argv);
		}
	else
		{
		strcpy(command,*++argv);
		if (command[0]=='-')
			{
			if (!strcmp(command,"-m")) 
				{
				pre_flag_v=1;
				strcpy(source,*++argv);
				strcpy(outfile,*++argv);
				}
			else if (!strcmp(command,"-c")) 
				{
				pre_flag_v=2;
				strcpy(source,*++argv);
				strcpy(outfile,*++argv);
				}
				else
				{
				printf("Unknown option '%s'\n",command);
				usage();
				exit(-1);
				}
			}
		else
			{
			usage();
			exit(-1);
			}
		}
    N=load_adev(source,tau,adev);
    if (N==-1)
        printf("# File not found\n");
    else
        {
        if (N<2)
	    {
            printf("# Unrecognized file\n\n");
	    usage();
	    }
        else
	    {
	    err=init_flag();
	    if (err==-1) printf("# ~/.SigmaTheta.conf not found, default values selected\n");
	    if (err==-2) printf("# ~/.SigmaTheta.conf improper, default values selected\n");
	    if (pre_flag_v==1) flag_variance=1;
	    else if (pre_flag_v==2) flag_variance=0;
	    for(i=0;i<N;++i) avar[i]=adev[i]*adev[i];
	    err=relatfit(N,tau,avar,tau,6);
            for(i=0;i<N;++i)
                {
                asympt=0;
/*              for(j=0;j<5;++j)
                    {
                    tsas=coeff[j]*interpo(tau[i],j);
                    if (tsas>asympt)
                        {
                        asympt=tsas;
                        if (j==0) alpha[i]=+2;
                        else alpha[i]=1-j;
                        }
                    }*/
		if (flag_variance)
			{
			for(j=0;j<5;++j) /* The drift is not concerned */
                    		{
                    		tsas=coeff[j]*interpo(tau[i],j);
                    		if (tsas>asympt)
                        		{
                        		asympt=tsas;
                         		alpha[i]=2-j;
                        		}
                    		}
			}
		else
			{
	                for(j=1;j<5;++j) /* Neither the tau^-3/2 asymptote nor the drift are concerned */
                    		{
                    		tsas=coeff[j]*interpo(tau[i],j);
                    		if (tsas>asympt)
                        		{
                        		asympt=tsas;
                        		if (j==1) alpha[i]=+2;
                        		else alpha[i]=2-j;
                        		}
                    		}
	                }
		}
	    avardof(N, tau, alpha, edf);
	    if (flag_fit&0x8)
	        {
	        for(i=0;i<N;++i) w[i]=((double)1)/edf[i];
	        err=relatfit(N,tau,avar,w,6);
                for(i=0;i<N;++i)
                    {
                    asympt=0;
		    if (flag_variance)
				{
				for(j=0;j<5;++j) /* The drift is not concerned */
        	            		{
        	            		tsas=coeff[j]*interpo(tau[i],j);
        	            		if (tsas>asympt)
        	                		{
        	                		asympt=tsas;
        	                 		alpha[i]=2-j;
        	                		}
        	            		}
				}
			else
				{
		                for(j=1;j<5;++j) /* Neither the tau^-3/2 asymptote nor the drift are concerned */
        	            		{
        	            		tsas=coeff[j]*interpo(tau[i],j);
        	            		if (tsas>asympt)
        	                		{
        	                		asympt=tsas;
        	                		if (j==1) alpha[i]=+2;
        	                		else alpha[i]=2-j;
        	                		}
        	            		}
		                }
/*                    for(j=0;j<4;++j)
                        {
                        tsas=coeff[j]*interpo(tau[i],j);
                        if (tsas>asympt)
                            {
                            asympt=tsas;
                            if (j==0) alpha[i]=+2;
                            else alpha[i]=1-j;
                            }
                        }*/
                    }
	        avardof(N, tau, alpha, edf);
	        }
	    ofd=fopen(outfile, "w");
	    if (flag_variance)
		{
	    	printf("# Tau       \t Mdev       \t Mdev unbiased\t 2.5 %% bound \t 16 %% bound \t 84 %% bound \t 97.5 %% bound\n");
	    	fprintf(ofd,"# Tau       \t Mdev       \t Mdev unbiased\t 2.5 %% bound \t 16 %% bound \t 84 %% bound \t 97.5 %% bound\n");
		}
	    else
		{
	    	printf("# Tau       \t Adev       \t Adev unbiased\t 2.5 %% bound \t 16 %% bound \t 84 %% bound \t 97.5 %% bound\n");
	    	fprintf(ofd,"# Tau       \t Adev       \t Adev unbiased\t 2.5 %% bound \t 16 %% bound \t 84 %% bound \t 97.5 %% bound\n");
		}
/*	    printf("# Tau       \t Adev       \t Adev unbiased\t 2.5 %% bound \t 16 %% bound \t 84 %% bound \t 97.5 %% bound\n");
	    fprintf(ofd,"# Tau       \t Adev       \t Adev unbiased\t 2.5 %% bound \t 16 %% bound \t 84 %% bound \t 97.5 %% bound\n");*/
	    for(i=0;i<N;++i)
	        {
		rayl=raylconfint(edf[i]);
		bmin[i]=adev[i]*sqrt(edf[i])/rayl.sup_bound;
		bmax[i]=adev[i]*sqrt(edf[i])/rayl.inf_bound;
		rayl=raylconfint1s(edf[i]);
		bi1s[i]=adev[i]*sqrt(edf[i])/rayl.sup_bound;
		bx1s[i]=adev[i]*sqrt(edf[i])/rayl.inf_bound;
		adc[i]=adev[i]/rayl.mean;
		printf("%12.6e \t %12.6e \t %12.6e \t %12.6e \t %12.6e \t %12.6e \t %12.6e\n",tau[i],adev[i],adc[i],bmin[i],bi1s[i],bx1s[i],bmax[i]);
		fprintf(ofd,"%12.6e \t %12.6e \t %12.6e \t %12.6e \t %12.6e \t %12.6e \t %12.6e\n",tau[i],adev[i],adc[i],bmin[i],bi1s[i],bx1s[i],bmax[i]);
		}
	    fclose(ofd);
	    if (flag_bias)
	      for(i=0;i<N;++i) avar[i]=adc[i]*adc[i];
	    for(i=0;i<N;++i) w[i]=((double)1)/edf[i];
	    err=relatfit(N,tau,avar,w,6);
	    printf("# Asymptote coefficients:\n");
	    printf("# tau^-3/2 \t tau^-1   \t tau^-1/2 \t tau^0    \t tau^1/2  \t tau^1\n");
	    for(i=0;i<6;++i) printf("%12.6e \t ",coeff[i]);
	    printf("\n");
/* Use of gnuplot for generating the graph as a ps file */
	    err=gener_gplt(outfile,N,tau,adev,bmax,"unbiased");
	    if (err) printf("# Error %d: ps file not created\n",err);
	    /*	    printf("tau: ");
	    for(i=0;i<N;++i) printf("%12.6e \t ",tau[i]);
	    printf("\n");
	    printf("edf: ");
	    for(i=0;i<N;++i) printf("%12.6e \t ",edf[i]);
	    printf("\n");*/
	    }
	}
    }
		  
