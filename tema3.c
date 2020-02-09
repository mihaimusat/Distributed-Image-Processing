/* Musat Mihai-Robert
 * Grupa 332CB
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#include "tema3.h"

int main(int argc, char *argv[])
{	
	int rank;
	int nProcesses;

    	MPI_Init(&argc, &argv);
    	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);	

	// imaginea de input in care voi stoca datele din fisier
	image img;

	// imaginea de output dupa aplicarea filtrelor
	image result;

	int i, j, u, v, nr;
	
	float filter[3][3];

	// variabile pentru impartirea matricii catre procese
	// start = linia de start pentru procesul curent
	// end = linia de final pentru procesul curent
	int start, end;

	// variabile pentru calculul sumei per pixel conform enuntului
	float sum, sum_r, sum_g, sum_b;
	float var, var_r, var_g, var_b;

	// verific sa am numarul de argumente corespunzatoare
	// argv[0] = numele executabilului
	// argv[1] = imagine de input
	// argv[2] = imagine de output
	// argv[3], argv[4], ... = filtrele aplicate 		
	if(argc < 4) {
		printf("Run program as: mpirun -np N ./%s image_in.pnm image_out.pnm filter1 filter2 ... filterX", argv[0]);
		exit(-1);
	}	

	// procesul cu rank 0 este "Master"
	// initial, acesta citeste datele de intrare si
	// le trimite mai departe celorlalte procese 
	if(rank == 0) {	
		
		// citire date de intrare din fisier
		// si stocare in img		
		readImage(argv[1], &img);
		
		// imaginea de output va avea 
		// aceiasi parametri ca imaginea de input
		result.type = img.type;
		result.height = img.height;
		result.width = img.width;
		result.maxval = img.maxval;

		// daca imaginea de input este alb-negru
		// atunci aloc matricea de pixeli gray[][] 
		if(img.type == 5) {
			result.gray = (gray_pixel **)malloc(result.height * sizeof(gray_pixel*));
			for(i = 0; i < result.height; i++) {
            			result.gray[i] = (gray_pixel*)malloc(result.width * sizeof(gray_pixel));
			}
		}
		
		// daca imaginea de input este color
		// atunci aloc matricea de pixeli color[][]
		if(img.type == 6) {
			result.color = (color_pixel **)malloc(result.height * sizeof(color_pixel*));
			for(i = 0; i < result.height; i++) {
            			result.color[i] = (color_pixel*)malloc(result.width * sizeof(color_pixel));
			}
		}

		// trimit de la procesul cu rank 0 la restul proceselor:
		// 1) parametrii imaginii de input
		// 2) partea din matrice corespunzatoare fiecarui proces (de la start la end)
		for(i = 1; i < nProcesses; i++) {
			MPI_Send(&img.height, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&img.width, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&img.type, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

			// calculez linia de start asociata procesului i
			start = i * ceil ((float) (img.height / nProcesses));
			
			// daca trimit ultimului proces
			// actualizez linia de final la care ma opresc
			if(i == nProcesses - 1) {
				end = img.height;
			}

			// altfel, calculez linia de final in functie 
			// de indicele procesului i
			else {
				end = (i + 1) * ceil ((float) (img.height / nProcesses));
			}
	
			// daca tipul imaginii este alb-negru
			// atunci trimit procesului i partea corespunzatoare 
			// din matricea de pixeli gray[][]
			if(img.type == 5) {
				for(j = start; j < end; j++) {
					MPI_Send(img.gray[j], img.width, MPI_CHAR, i, 0, MPI_COMM_WORLD);
				}
			}
			
			// daca tipul imaginii este color
			// atunci trimit procesului i partea corespunzatoare 
			// din matricea de pixeli color[][]
			if(img.type == 6) {
				for(j = start; j < end; j++) {
					MPI_Send(img.color[j], img.width * 3, MPI_CHAR, i, 0, MPI_COMM_WORLD);
				}
			}
			
		}	
	
	}
		
	// daca procesul nu are rank 0, atunci va avea rolul de "Worker"
	// 1) primeste parametrii imaginii de input de la procesul cu rank 0
	// 2) primeste partea de matrice de procesat de la procesul cu rank 0
	else {
		MPI_Recv(&img.height, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&img.width, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv(&img.type, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		// calculez linia de start pentru un proces in functie de rank 
		start = rank * ceil ((float) (img.height / nProcesses));
	
		// daca trimit ultimului proces
		// actualizez linia de final la care ma opresc
		if(rank == nProcesses - 1) {
			end = img.height;
		}
	
		// altfel, calculez linia de final in functie de rank-ul procesului
		else {
			end = (rank + 1) * ceil ((float) (img.height / nProcesses));
		}

		// daca tipul imaginii este alb-negru
		// primesc partea de matrice corespunzatoare de la procesul cu rank 0
		// dupa ce am facut alocarile de memorie necesare
		if(img.type == 5) {

			img.gray = (gray_pixel **)malloc(img.height * sizeof(gray_pixel*));
			for(i = 0; i < img.height; i++) {
            			img.gray[i] = (gray_pixel*)malloc(img.width * sizeof(gray_pixel));
			}

			result.gray = (gray_pixel **)malloc(img.height * sizeof(gray_pixel*));
			for(i = 0; i < img.height; i++) {
            			result.gray[i] = (gray_pixel*)malloc(img.width * sizeof(gray_pixel));
			}
			
			for(j = start; j < end; j++) {
				MPI_Recv(img.gray[j], img.width, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}
		
		// daca tipul imaginii este color
		// primesc partea de matrice corespunzatoare de la procesul cu rank 0
		// dupa ce am facut alocarile de memorie necesare
		if(img.type == 6) {
		
			img.color = (color_pixel **)malloc(img.height * sizeof(color_pixel*));
			for(i = 0; i < img.height; i++) {
            			img.color[i] = (color_pixel*)malloc(img.width * sizeof(color_pixel));
			}

			result.color = (color_pixel **)malloc(img.height * sizeof(color_pixel*));
			for(i = 0; i < img.height; i++) {
            			result.color[i] = (color_pixel*)malloc(img.width * sizeof(color_pixel));
			}
			
			for(j = start; j < end; j++) {
				MPI_Recv(img.color[j], img.width * 3, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
		}
	}
	
	
	// in acest punct, pot sa iau toate filtrele in ordine
	// si sa initializez matricea filter cu unul din cele 5 filtre
	// iar apoi sa aplic filtrul pe imaginea de input
	for(nr = 3; nr < argc; nr++) {

		int filter_type = getFilter(argv[nr]);

		if(filter_type == 1) {
			for(i = 0; i < 3; i++) {
				for(j = 0; j < 3; j++) {
					filter[i][j] = SMOOTH_FILTER[i][j];
				}
			}
		}

		if(filter_type == 2) {
			for(i = 0; i < 3; i++) {
				for(j = 0; j < 3; j++) {
					filter[i][j] = BLUR_FILTER[i][j];
				}
			}
			
		}

		if(filter_type == 3) {
			for(i = 0; i < 3; i++) {
				for(j = 0; j < 3; j++) {
					filter[i][j] = SHARPEN_FILTER[i][j];
				}
			}
		}

		if(filter_type == 4) {
			for(i = 0; i < 3; i++) {
				for(j = 0; j < 3; j++) {
					filter[i][j] = MEAN_REMOVAL_FILTER[i][j];
				}
			}
		}

		if(filter_type == 5) {
			for(i = 0; i < 3; i++) {
				for(j = 0; j < 3; j++) {
					filter[i][j] = EMBOSS_FILTER[i][j];
				}
			}
		}

		// rotesc matricea cu 180 de grade pentru a obtine
		// valoarea corecta a sumei din enunt
		rotate_180(filter);

		// daca rank-ul procesului curent nu este 0
		// calculez linia de start in functie de rank
		if(rank != 0) {
			start = rank * ceil ((float) (img.height / nProcesses));
		}
	
		// altfel, linia de start va fi prima linie
		else {
			start = 0;
		}
		
		// daca rank-ul procesului curent este chiar ultimul proces
		// linia de final pe care o va prelucra va fi ultima linie
		if(rank == nProcesses - 1) {
			end = img.height;
		}

		// altfel, linia de final o voi calcula in functie de rank
		else {
			end = (rank + 1) * ceil ((float) (img.height / nProcesses));
		}

		// calculez width in functie de tipul imaginii
		int w;

		if(img.type == 5) {
			w = img.width;
		}
		
		if(img.type == 6) {
			w = img.width * 3;
		}

		// daca imaginea este alb-negru
		if(img.type == 5) {

			// trimit si primesc linia de la vecini in functie de 
			// rank-ul procesului curent
			if(rank > 0) {
				int last = start - 1;            
                		MPI_Sendrecv(img.gray[start], w, MPI_CHAR, rank - 1, 0,    
					     img.gray[last], w, MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, NULL);
			}

			if(rank < nProcesses - 1) {
				int first = end - 1;
				MPI_Sendrecv(img.gray[first], w, MPI_CHAR, rank + 1, 0, 
					     img.gray[end], w, MPI_CHAR, rank + 1, 0, MPI_COMM_WORLD, NULL);
			}

			// aplic filtrul doar pe liniile care ma intereseaza
			for(i = start; i < end; i++) {
				for(j = 0; j < img.width; j++) {
					sum = 0; 
					for(u = i - 1; u <= i + 1; u++) {
                        			for(v = j - 1; v <= j + 1; v++) {
							if(! (u < 0 || u >= img.height || v < 0 || v >= img.width) ) { 	
								var = img.gray[u][v].val * filter[u - i + 1][v - j + 1];
							        sum += var;
							}
                        			}
                    			}

					// verific daca suma calculata face overflow sau underflow
					if(sum > 255.f) {
						sum = 255.f;
					}
							
					if(sum < 0.f) {
						sum = 0.f;
					}

					// actualizez valoarea din matrice cu suma calculata
					result.gray[i][j].val = (unsigned char) sum;
				}
			}

			// dupa ce am aplicat filtrul, copiez matricea obtinuta in img
			// pentru a putea sa ii aplic filtrul urmator
			for(i = start; i < end; i++) {
				for(j = 0; j < img.width; j++) {
                    			img.gray[i][j].val = result.gray[i][j].val;
                		}
            		}
		}

		// daca imaginea este color
		if(img.type == 6) {
			
			// trimit si primesc linia de la vecini in functie de 
			// rank-ul procesului curent
			if(rank > 0) {
				int last = start - 1;            
                		MPI_Sendrecv(img.color[start], w, MPI_CHAR, rank - 1, 0,    
					     img.color[last], w, MPI_CHAR, rank - 1, 0, MPI_COMM_WORLD, NULL);
			}

			if(rank < nProcesses - 1) {
				int first = end - 1;
				MPI_Sendrecv(img.color[first], w, MPI_CHAR, rank + 1, 0, 
					     img.color[end], w, MPI_CHAR, rank + 1, 0, MPI_COMM_WORLD, NULL);
			}

			// aplic filtrul doar pe liniile care ma intereseaza
			for(i = start; i < end; i++) {
				for(j = 0; j < img.width; j++) {
					sum_r = 0;
					sum_g = 0;
					sum_b = 0; 
					for(u = i - 1; u <= i + 1; u++) {
                        			for(v = j - 1; v <= j + 1; v++) {
							if(! (u < 0 || u >= img.height || v < 0 || v >= img.width) ) {
								var_r = img.color[u][v].r * filter[u - i + 1][v - j + 1];
								var_g = img.color[u][v].g * filter[u - i + 1][v - j + 1];
								var_b = img.color[u][v].b * filter[u - i + 1][v - j + 1];
                            					sum_r += var_r;
								sum_g += var_g;
								sum_b += var_b;
							}
                        			}
                    			}

					// verific ca sumele calculate sa nu faca overflow sau underflow
					if(sum_r > 255.f) {
						sum_r = 255.f;
					}
							
					if(sum_r < 0.f) {
						sum_r = 0.f;
					}

					if(sum_g > 255.f) {
						sum_g = 255.f;
					}
							
					if(sum_g < 0.f) {
						sum_g = 0.f;
					}
						
					if(sum_b > 255.f) {
						sum_b = 255.f;
					}
							
					if(sum_b < 0.f) {
						sum_b = 0.f;
					}
					
					// actualizez valoarea din matrice in functie de tipul de pixel
					// cu suma corespunzatoare
					result.color[i][j].r = (unsigned char) sum_r;
					result.color[i][j].g = (unsigned char) sum_g;
					result.color[i][j].b = (unsigned char) sum_b;
				}
			}

			// dupa ce am aplicat filtrul, copiez matricea obtinuta in img
			// pentru a putea sa ii aplic filtrul urmator
			for(i = start; i < end; i++) {
				for(j = 0; j < img.width; j++) {
                    			img.color[i][j].r = result.color[i][j].r;
					img.color[i][j].g = result.color[i][j].g;
					img.color[i][j].b = result.color[i][j].b;
                		}
            		}
		}
	}

	// dupa ce am aplicat filtrele, daca rank-ul procesului curent
	// este diferit de 0 (este un proces "Worker"), atunci trebuie sa
	// ii trimita procesului cu rank = 0 partea din matrice modificata
	if(rank != 0) {

		// calculez linia de start in functie de rank
		start = rank * ceil ((float) (img.height / nProcesses));

		// daca rank-ul procesului curent este chiar ultimul proces
		// linia de final pe care o va trimite va fi ultima linie
		if(rank == nProcesses - 1) {
			end = img.height;
		}
		
		// altfel, linia de final o voi calcula in functie de rank
		else {
			end = (rank + 1) * ceil ((float) (img.height / nProcesses));
		}
	
		// daca tipul imaginii este alb-negru
		// trimit matricea de pixeli gray[][] procesului cu rank = 0
		if(img.type == 5) {
			for(j = start; j < end; j++) {
				MPI_Send(result.gray[j], img.width, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
			}
		}

		// daca tipul imaginii este color
		// trimit matricea de pixeli color[][] procesului cu rank = 0
		if(img.type == 6) {
			for(j = start; j < end; j++) {
				MPI_Send(result.color[j], img.width * 3, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
			}
		}
	}

	// daca procesul curent are rank = 0 (este procesul "Master"), atunci 
	// primeste de la fiecare proces partea din matrice pe care a modificat-o
	// si construieste rezultatul final
	else {
		for(i = 1; i < nProcesses; i++) {

			// calculeaza linia de start in functie de indexul i al procesului
			start = i * ceil ((float) (img.height / nProcesses));

			// daca rank-ul procesului curent este chiar ultimul proces
			// linia de final pe care o va trimite va fi ultima linie 
			if(i == nProcesses - 1) {
				end = img.height;
			}
			
			// altfel, linia de final o voi calcula in functie de indexul i al procesului
			else {
				end = (i + 1) * ceil ((float) (img.height / nProcesses));
			}

			// daca imaginea este alb-negru
			// atunci procesul i trimite partea din matricea gray[][] pe care a prelucrat-o
			if(img.type == 5) {
				for(j = start; j < end; j++) {
					MPI_Recv(result.gray[j], result.width, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}
			}

			// daca imaginea este color
			// atunci procesul i trimite partea din matricea color[][] pe care a prelucrat-o
			if(img.type == 6) {
				for(j = start; j < end; j++) {
					MPI_Recv(result.color[j], result.width * 3, MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				}
			}
		}
		
		// scrie imaginea finala obtinuta in result in fisier
		writeImage(argv[2], &result);
	}
			
	MPI_Finalize();

	return 0;
}
