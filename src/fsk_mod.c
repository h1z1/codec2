/*---------------------------------------------------------------------------*\

  FILE........: fsk_mod.c
  AUTHOR......: Brady O'Brien and David Rowe
  DATE CREATED: 8 January 2016

  Command line FSK modulator.  Reads in bits, writes FSK modulated output.
   
\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2016 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License version 2.1, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "fsk.h"
#include "codec2_fdmdv.h"

int main(int argc,char *argv[]){
    struct FSK *fsk;
    int Fs,Rs,f1,fs,M;
    int i;
    int p, user_p = 0;
    FILE *fin,*fout;
    uint8_t *bitbuf;
    int16_t *rawbuf;
    float *modbuf;
    
    char usage[] = "usage: %s [-p P] Mode SampleFreq SymbolFreq TxFreq1 TxFreqSpace InputOneBitPerCharFile OutputModRawFile\n";

    int opt;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            p = atoi(optarg);
            user_p = 1;
            break;
        default:
            fprintf(stderr, usage, argv[0]);
            exit(1);
        }
    }

    if (argc<8){
        fprintf(stderr, usage, argv[0]);
        exit(1);
    }
    
    /* Extract parameters */
    M = atoi(argv[optind++]);
    Fs = atoi(argv[optind++]);
    Rs = atoi(argv[optind++]);
    f1 = atoi(argv[optind++]);
    fs = atoi(argv[optind++]);
    
    if(strcmp(argv[optind],"-")==0){
        fin = stdin;
    }else{
        fin = fopen(argv[optind],"r");
    }
    optind++;
    
    if(strcmp(argv[optind],"-")==0){
        fout = stdout;
    }else{
        fout = fopen(argv[optind],"w");
    }

    /* p is not actually used for the modulator, but we need to set it for fsk_create() to be happy */    
    if (!user_p)
        p = Fs/Rs;
    
    /* set up FSK */
    fsk = fsk_create_hbr(Fs,Rs,M,p,FSK_DEFAULT_NSYM,f1,fs);
    
    if(fin==NULL || fout==NULL || fsk==NULL){
        fprintf(stderr,"Couldn't open test vector files\n");
        goto cleanup;
    }
    
    
    /* allocate buffers for processing */
    bitbuf = (uint8_t*)malloc(sizeof(uint8_t)*fsk->Nbits);
    rawbuf = (int16_t*)malloc(sizeof(int16_t)*fsk->N);
    modbuf = (float*)malloc(sizeof(float)*fsk->N);
    
    /* Modulate! */
    while( fread(bitbuf,sizeof(uint8_t),fsk->Nbits,fin) == fsk->Nbits ){
        fsk_mod(fsk,modbuf,bitbuf);
        for(i=0; i<fsk->N; i++){
			rawbuf[i] = (int16_t)(modbuf[i]*(float)FDMDV_SCALE);
		}
        fwrite(rawbuf,sizeof(int16_t),fsk->N,fout);
        
		if(fin == stdin || fout == stdin){
			fflush(fin);
			fflush(fout);
		}
    }
    free(bitbuf);
    free(rawbuf);
    free(modbuf);
    
    cleanup:
    fclose(fin);
    fclose(fout);
    fsk_destroy(fsk);
    exit(0);
}
