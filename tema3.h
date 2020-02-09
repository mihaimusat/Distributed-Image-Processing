/* Musat Mihai-Robert
 * Grupa 332CB
 */

#ifndef TEMA3_H
#define TEMA3_H

// structura pentru un pixel color
typedef struct {
	unsigned char r, g, b;
} color_pixel;

// structura pentru un pixel alb-negru
typedef struct {
	unsigned char val;
} gray_pixel;

// structura unei imagini asa cum este descrisa in enunt,
// are un header si o matrice de pixeli in functie de tipul imaginii 
typedef struct {
	int type;
	int height;
	int width;
	unsigned char maxval;
	gray_pixel** gray;
	color_pixel** color;
} image;

// declarare matrici pentru fiecare tip de filtru
float SMOOTH_FILTER[3][3] = {{1.f/9, 1.f/9, 1.f/9},
                             {1.f/9, 1.f/9, 1.f/9},
                             {1.f/9, 1.f/9, 1.f/9}};

float BLUR_FILTER[3][3] = {{1.f/16, 2.f/16, 1.f/16},
                           {2.f/16, 4.f/16, 2.f/16},
                           {1.f/16, 2.f/16, 1.f/16}};

float SHARPEN_FILTER[3][3] = {{0.f, -2.f/3, 0.f},
                              {-2.f/3, 11.f/3, -2.f/3},
                              {0.f, -2.f/3, 0.f}};

float MEAN_REMOVAL_FILTER[3][3] = {{-1.f, -1.f, -1.f},
                             	   {-1.f, 9.f, -1.f},
                                   {-1.f, -1.f, -1.f}};

float EMBOSS_FILTER[3][3] = {{0.f, 1.f, 0.f},
                             {0.f, 0.f, 0.f},
                             {0.f, -1.f, 0.f}};

// functie auxiliara care intoarce tipul filtrului
// in functie de numele dat ca parametru
int getFilter(char* filter_name) 
{
	if(strcmp(filter_name, "smooth") == 0) {
		return 1;
	}

	if(strcmp(filter_name, "blur") == 0) {
		return 2;
	}

	if(strcmp(filter_name, "sharpen") == 0) {
		return 3;
	}

	if(strcmp(filter_name, "mean") == 0) {
		return 4;
	}

	if(strcmp(filter_name, "emboss") == 0) {
		return 5;
	}
}

// functie auxiliara care roteste cu 180 de grade o matrice 3 x 3
void rotate_180(float mat[3][3]) 
{	
	int N = 3;

	int i, j;
	
	// rotire matrice cu 180 de grade
	for(i = 0; i < N / 2; i++) {
		for(j = 0; j < N; j++) {
			float aux = mat[i][j];
			mat[i][j] = mat[N - i - 1][N - j - 1];
			mat[N - i - 1][N - j - 1] = aux;
		}
	}

	// tratez separat cazul in care matricea are dimensiunea N impara
	if(N & 1) {
		for(j = 0; j < N / 2; j++) {
			float aux = mat[N / 2][j];
			mat[N / 2][j] = mat[N / 2][N - j - 1];
			mat[N / 2][N - j - 1] = aux;
		}
	}
}

// functie care citeste date dintr-un fisier
// care are numele "filename" si le salveaza
// intr-o structura de tip image "img"
void readImage(const char* filename, image* img) 
{
	// deschidere fisier de intrare
	FILE* fin = fopen(filename, "rb");
	if(fin == NULL) {
		printf("Could not open file %s\n", filename);
		return;
	}

	// pentru a obtine tipul imaginii folosesc un buffer
	// in care retin urmatoarele 2 caractere din fisier
	char arr[2];
    	fscanf(fin, "%c%c\n", &arr[0], &arr[1]);
	
	// fac cast la int pentru a obtine type = 5 sau type = 6
    	img->type = arr[1] - '0';

	// citire linie de comentariu
	fscanf(fin, "# Created by GIMP version 2.10.14 PNM plug-in\n");
	
	// citire width, height si maxval
	fscanf(fin, "%d %d\n%hhu\n", &img->width, &img->height, &img->maxval);

	int i;
	
	// daca tipul imaginii este alb-negru
	// atunci aloc matricea de pixeli si o umplu cu valorile din fisier
	if(img->type == 5) {
		img->gray = (gray_pixel **)malloc(img->height * sizeof(gray_pixel*));
		for(i = 0; i < img->height; i++) {
            		img->gray[i] = (gray_pixel*)malloc(img->width * sizeof(gray_pixel));
			fread(img->gray[i], sizeof(gray_pixel), img->width, fin);
		}
	}
	
	// daca tipul imaginii este color
	// atunci aloc matricea de pixeli si o umplu cu valorile din fisier
	if(img->type == 6) {
		img->color = (color_pixel **)malloc(img->height * sizeof(color_pixel*));
		for(i = 0; i < img->height; i++) {
            		img->color[i] = (color_pixel*)malloc(img->width * sizeof(color_pixel));
			fread(img->color[i], sizeof(color_pixel), img->width, fin);
		}
		
	}
	
	// inchidere fisier de intrare
	fclose(fin);
}

// functie care scrie datele obtinute dintr-o
// structura de tip image "img" intr-un
// fisier avand numele "filename" 
void writeImage(const char* filename, image* img) 
{
	// deschidere fisier de iesire
	FILE* fout = fopen(filename, "wb");
	if(fout == NULL) {
		printf("Could not open file %s\n", filename);
		return;
	}

	// scriere tip imagine
	fprintf(fout, "P%d\n", img->type);

	// scriere width, height, maxval
	fprintf(fout, "%d %d\n%hhu\n", img->width, img->height, img->maxval);

	int i, j;

	// daca tipul imaginii este alb-negru
	// atunci scriu matricea de pixeli gray[][] in fisierul de iesire
	if(img->type == 5) {
		for(i = 0; i < img->height; i++) {
			for(j = 0; j < img->width; j++) {
				fwrite(&img->gray[i][j], sizeof(unsigned char), 1, fout);
			}
		}
	}

	// daca tipul imaginii este color
	// atunci scriu matricea de pixeli color[][] in fisierul de iesire
	if(img->type == 6) {
		for(i = 0; i < img->height; i++) {
			for(j = 0; j < img->width; j++) {
				fwrite(&img->color[i][j], sizeof(unsigned char), 3, fout);
			}
		}
	}
	
	// inchidere fisier de iesire
	fclose(fout);
}

#endif /* TEMA3_H */
