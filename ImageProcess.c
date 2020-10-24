#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<float.h>
#include<string.h>
#include<ctype.h>

struct pgmFile{
    unsigned char** pgmMatrix;
    char type[2];
    int column;
    int row;
    int grayscale;
};

typedef struct pgmFile pgmData;

void skipCommentLine(FILE *fp){
    int ch;
    char line[100];
    while((ch=fgetc(fp))!=EOF && isspace(ch)); //It will check every character inside
    if(ch=='#'){    //If it is equals to '#'
        fgets(line,sizeof(line),fp); // Skipp line
        skipCommentLine(fp);    //And check again
    }else{
        fseek(fp,-1,SEEK_CUR);  //If not move the cursor back 1 unit
    }
}

void deallocateMemoryUC(unsigned char** matrix,int row){
    int i;
    for(i=0;i<row;i++){
        free(matrix[i]);
    }
    free(matrix);
}

void deallocateMemoryS(short** matrix, int row){
    int i;
    for(i=0;i<row;i++){
        free(matrix[i]);
    }
    free(matrix);
}

unsigned char** allocateMemoryUC(int row,int column){
    unsigned char** matrix;
    int i;

    matrix=(unsigned char**)calloc(row,sizeof(unsigned char*));
    if(matrix==NULL){
        printf("Calloc Error");
        exit(EXIT_FAILURE);
    }

    for(i=0;i<row;i++){
        matrix[i]=(unsigned char*)calloc(column,sizeof(unsigned char));
        if(matrix[i]==NULL){
            printf("Calloc Error");
            exit(EXIT_FAILURE);
        }
    }

    return matrix;
}

short** allocateMemoryS(int row,int column){
    short** matrix;
    int i;

    matrix=(short**)malloc(row*sizeof(short*));
    if(matrix==NULL){
        printf("Malloc Error");
        exit(EXIT_FAILURE);
    }

    for(i=0;i<row;i++){
        matrix[i]=(short*)malloc(column*sizeof(short));
        if(matrix[i]==NULL){
            printf("Malloc Error");
            exit(EXIT_FAILURE);
        }
    }

    return matrix;
}

int main(){
    char filename[30];  //Holding file name to open it
    FILE *ptr;          //Reading data from pgm file and and writing data new pgm file
    pgmData image;      //pgm object
    short **x_matrix,**y_matrix,**xy_matrix;    //For working on
    unsigned char **mtrX,**mtrY,**mtrXY;        //For holding last values of each gradient
    int i,j;            // For Looping iterators
    double maxX=-DBL_MAX,minX=DBL_MAX,maxY=-DBL_MAX,minY=DBL_MAX,maxXY=-DBL_MAX,minXY=DBL_MAX; // For Holding max and min value of each gradient


    printf("Ayni dizindeki dosyanin adini giriniz: ");
    scanf("%s",&filename);

    ptr=fopen(filename,"rb");

    skipCommentLine(ptr);
    fscanf(ptr,"%s\n",&image.type);
    skipCommentLine(ptr);
    fscanf(ptr,"%d %d\n",&image.column,&image.row);
    skipCommentLine(ptr);
    fscanf(ptr,"%d\n",&image.grayscale);

    image.pgmMatrix=allocateMemoryUC(image.row,image.column);

    //after applying filter we will lose data. For N*N => (N-1)*(N-1)
    x_matrix=allocateMemoryS(image.row-1,image.column-1);
    y_matrix=allocateMemoryS(image.row-1,image.column-1);
    xy_matrix=allocateMemoryS(image.row-1,image.column-1);

    //The reason of calloc() is to surround of the last matrix with 0
    mtrX=allocateMemoryUC(image.row,image.column);
    mtrY=allocateMemoryUC(image.row,image.column);
    mtrXY=allocateMemoryUC(image.row,image.column);

    for(i=0;i<image.row;i++){
        fread(image.pgmMatrix[i],sizeof(unsigned char),image.column,ptr);
        if(ferror(ptr)){
            printf("fread Reading Error");
            exit(EXIT_FAILURE);
        }
    }
    fclose(ptr);

    // SOBEL X and Y together
    for(i=0;i<image.row-2;i++){
        for(j=0;j<image.column-2;j++){
            x_matrix[i][j]=image.pgmMatrix[i][j+2]+image.pgmMatrix[i+2][j+2]-image.pgmMatrix[i][j]-image.pgmMatrix[i+2][j]+2*(image.pgmMatrix[i+1][j+2]-image.pgmMatrix[i+1][j]);
            y_matrix[i][j]=image.pgmMatrix[i+2][j]+image.pgmMatrix[i+2][j+2]-image.pgmMatrix[i][j]-image.pgmMatrix[i][j+2]+2*(image.pgmMatrix[i+2][j+1]-image.pgmMatrix[i][j+1]);
            if(x_matrix[i][j]>maxX){
                maxX=x_matrix[i][j];
            }else if(x_matrix[i][j]<minX){
                minX=x_matrix[i][j];
            }
            if(y_matrix[i][j]>maxY){
                maxY=y_matrix[i][j];
            }else if(y_matrix[i][j]<minY){
                minY=y_matrix[i][j];
            }
        }
    }
    //Min-Max normalization for X and Y gradient
    for(i=0;i<image.row-2;i++){
        for(j=0;j<image.column-2;j++){
            mtrX[i+1][j+1]=round((image.grayscale*(x_matrix[i][j]-minX))/(maxX-minX));
            mtrY[i+1][j+1]=round((image.grayscale*(y_matrix[i][j]-minY))/(maxY-minY));
        }
    }

    ptr=fopen("X-Out.pgm","wb");

    fprintf(ptr,"%s\n",image.type);
    fprintf(ptr,"%d %d\n",image.column,image.row);
    fprintf(ptr,"%d\n",image.grayscale);

    for(i=0;i<image.row;i++){
        fwrite(mtrX[i],sizeof(unsigned char),image.column,ptr);
    }

    fclose(ptr);

    ptr=fopen("Y-Out.pgm","wb");

    fprintf(ptr,"%s\n",image.type);
    fprintf(ptr,"%d %d\n",image.column,image.row);
    fprintf(ptr,"%d\n",image.grayscale);

    for(i=0;i<image.row;i++){
        fwrite(mtrY[i],sizeof(unsigned char),image.column,ptr);
    }

    fclose(ptr);

    //XY Gradient
    for(i=0;i<image.row-2;i++){
        for(j=0;j<image.column-2;j++){
            xy_matrix[i][j]=round(sqrt(mtrX[i+1][j+1]*mtrX[i+1][j+1]+mtrY[i+1][j+1]*mtrY[i+1][j+1]));
            if(xy_matrix[i][j]>maxXY){
                maxXY=xy_matrix[i][j];
            }else if(xy_matrix[i][j]<minXY){
                minXY=xy_matrix[i][j];
            }
        }
    }
    for(i=0;i<image.row-2;i++){
        for(j=0;j<image.column-2;j++){
            mtrXY[i+1][j+1]=round((image.grayscale*(xy_matrix[i][j]-minXY))/(maxXY-minXY));
        }
    }
    ptr=fopen("XY-Out.pgm","wb");

    fprintf(ptr,"%s\n",image.type);
    fprintf(ptr,"%d %d\n",image.column,image.row);
    fprintf(ptr,"%d\n",image.grayscale);

    for(i=0;i<image.row;i++){
        fwrite(mtrXY[i],sizeof(unsigned char),image.column,ptr);
    }

    fclose(ptr);

    maxXY=-DBL_MAX;
    minXY=DBL_MAX;
    for(i=0;i<image.row;i++){
        for(j=0;j<image.column;j++){
            if(mtrXY[i][j]>maxXY){
                maxXY=mtrXY[i][j];
            }else if(mtrXY[i][j]<minXY){
                minXY=mtrXY[i][j];
            }
        }
    }

    for(i=0;i<image.row;i++){
        for(j=0;j<image.column;j++){
            if(mtrXY[i][j]<(maxXY+minXY)/2){
                mtrXY[i][j]=0;
            }else{
                mtrXY[i][j]=255;
            }
        }
    }
    ptr=fopen("XY-OutThreshold.pgm","wb");

    fprintf(ptr,"%s\n",image.type);
    fprintf(ptr,"%d %d\n",image.column,image.row);
    fprintf(ptr,"%d\n",image.grayscale);

    for(i=0;i<image.row;i++){
        fwrite(mtrXY[i],sizeof(unsigned char),image.column,ptr);
    }

    fclose(ptr);

    deallocateMemoryUC(mtrXY,image.row);
    deallocateMemoryUC(mtrY,image.row);
    deallocateMemoryUC(mtrX,image.row);
    deallocateMemoryS(xy_matrix,image.row);
    deallocateMemoryS(y_matrix,image.row);
    deallocateMemoryS(x_matrix,image.row);
    deallocateMemoryUC(image.pgmMatrix,image.row);

    return 0;
}













