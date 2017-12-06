/*From
https://sites.google.com/site/itstudentjunction/lab-programming-solutions/data-structures-programs/trees--programs---data-structures/program-for-implementation-of-b-tree-insertion--deletion
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define M 5   // Ordem da arvore
#define TAM_BLOCO 64 // blocos serão de tamanho 64 bytes
#define TAM_CABECALHO 8 // cabecalho terá tamanho 8 bytes

// Possui 64 bytes
struct node {
    int n; /* n < M No. of keys in node will always less than order of B tree */ // Quantidade de chave no nó
    int keys[M-1]; /*array of keys*/ //chaves
    long int p[M]; // apontarão para a posiçao dos filhos do nó no arquivo 
};

//Estrutura para o bloco contendo apenas um nó (64 bytes)
struct Bloco{
	  struct node no;
};

//Estrutura para o cabecalho contendo o long int que aponta para posicao da root no arquivo (tamanho 8)
struct Cabecalho{
    long int root;
};


enum KeyStatus { Duplicate,SearchFailure,Success,InsertIt,LessKeys, FailedOpenFile };

int insert(int key);
void display(long int root,int);
//void DelNode(int x);
void search(int x);
enum KeyStatus ins(long int r, int x, int* y, struct node** u, long int *newNodePos);
int searchPos(int x,int *key_arr, int n);
// enum KeyStatus del(struct node *r, int x);
void eatline(void);
// void inorder(struct node *ptr);
// int totalKeys(struct node *ptr);
// void printTotal(struct node *ptr);
// int getMin(struct node *ptr);
// int getMax(struct node *ptr);
// void getMinMax(struct node *ptr);
// int max(int first, int second, int third);
// int maxLevel(struct node *ptr);
//void printMaxLevel(struct node *ptr);
void criarArquivo(FILE* arquivo);
struct Bloco* criarBloco();
struct Cabecalho* criarCabecalho();


int main()
{
    FILE *arquivo;
    struct Bloco bloco;
    struct Cabecalho *cabecalho = criarCabecalho();
    int key;
    int choice;
    printf("Creation of B tree for M=%d\n",M);
    while(1)
    {
        // Abre arquivo em modo leitura
        arquivo = fopen("arquivo.txt", "rb");

        // Se arquivo foi aberto com sucesso
        if(arquivo){
            // Le cabecalho
            fseek(arquivo, 0, SEEK_SET);
            fread(cabecalho, TAM_CABECALHO, 1, arquivo);
            //Fecha arquivo
            
        }

        // Fecha arquivo
        fclose(arquivo);

        printf("\n\n0.New File\n");
        printf("1.Insert\n");
        // printf("2.Delete\n");
        printf("3.Search\n");
        printf("4.Display\n");
        // printf("5.Quit\n");
        // printf("6.Enumerate\n");
        // printf("7.Total Keys\n");
        // printf("8.Min and Max Keys\n");
        // printf("9.Max Level\n");
        printf("Enter your choice : ");
        scanf("%d",&choice); eatline();

        switch(choice)
        {
        case 0:
            criarArquivo(arquivo);
            break;
        case 1:
            printf("Enter the key : ");
            scanf("%d",&key); eatline();
            insert(key);
            break;
        // case 2:
        //     printf("Enter the key : ");
        //     scanf("%d",&key); eatline();
        //     DelNode(key);
        //     break;
        case 3:
            printf("Enter the key : ");
            scanf("%d",&key); eatline();
            search(key);
            break;
        case 4:
            printf("Btree is :\n");
            display(cabecalho->root,0);
            break;
        // case 5:
        //     exit(1);
        // case 6:
        //     printf("Btree in sorted order is:\n");
        //     inorder(root); putchar('\n');
        //     break;
        // case 7:
        // 	printf("The total number of keys in this tree is:\n");
        //     printTotal(root);
        //     break;
        // case 8:
        //     getMinMax(root);
        //     break;
        // case 9:
        // 	printf("The maximum level in this tree is:\n");
        // 	printMaxLevel(root);
        // 	break;
        default:
            printf("Wrong choice\n");
            break;
        }/*End of switch*/
    }/*End of while*/
    return 0;
}/*End of main()*/

int insert(int key)
{
    // Cria FILE arquivo
    FILE *arquivo;
    // Cria Bloco bloco
    struct Bloco* bloco = criarBloco();
    // Cria Cabecalho cabecalho
    struct Cabecalho* cabecalho = criarCabecalho();

    // Abre arquivo em modo de leitura e escrita
    arquivo = fopen("arquivo.txt", "rb+");

    // Verifica se arquivo foi aperto com sucesso
    if(!arquivo){
        printf("\nFailed to open file\n");
        return 0;
    }
    else{
        // Posiciona seek no inicio do arquivo
        fseek(arquivo, 0, SEEK_SET);
        // Le o cabecalho
        fread(cabecalho, TAM_CABECALHO, 1, arquivo);
        // Fecha o arquivo
        fclose(arquivo);
        long int newNodePos;
        struct node *newnode;  // Cria novo nó
        int upKey;
        enum KeyStatus value;
        value = ins(cabecalho->root, key, &upKey, &newnode, &newNodePos);
        if (value == Duplicate)
            printf("\nKey already available\n");
        if (value == InsertIt)
        {
            // Abre arquivo em modo de leitura e escrita
            arquivo = fopen("arquivo.txt", "rb+");

            long int uproot = cabecalho->root;

             // Posiciona seek no final do arquivo
            fseek(arquivo, 0, SEEK_END);

            // A raiz agora salva em cabecalho vai apontar para o final do arquivo
            cabecalho->root = ftell(arquivo);
            
            // Coloca informações do novo bloco
            bloco->no.n = 1;
            bloco->no.keys[0] = upKey;
            bloco->no.p[0] = uproot;
            bloco->no.p[1] = newNodePos;

            // Insere novo bloco no fim do arquivo
            fwrite(bloco, TAM_BLOCO, 1, arquivo);

            // Posiciona seek no começo do arquivo
            fseek(arquivo, 0, SEEK_SET);
            // Reescreve cabecalho atualizado
            fwrite(cabecalho, TAM_CABECALHO, 1, arquivo);

            // Fecha arquivo
            free(bloco);
            free(cabecalho);
            fclose(arquivo);
        }/*End of if */

        return 1;
    }

}/*End of insert()*/

enum KeyStatus ins(long int ptr, int key, int *upKey,struct node **newnode, long int *newNodePos)
{
    // ler o que está na raiz
    FILE *arquivo;
    struct Bloco* blocoPtr = criarBloco();
    struct node *newPtr;
    long int newPtrPos, lastPtrPos;
    int pos, i, n,splitPos;
    int newKey, lastKey;
    enum KeyStatus value;


    // Abre arquivo em modo leitura e escrita
    arquivo = fopen("arquivo.txt", "rb+");

    // Verifica se arquivo foi aberto corretamente
    if(!arquivo){
        printf("\nFailed to open file no ins\n");
        return FailedOpenFile;
    }
    

    if (ptr == -1)  // Verifica se encontrou onde inserir para criar um novo nó (ptr sendo -1 indica nulo)
    {
        *newnode = NULL;
        *newNodePos = -1;
        *upKey = key;
        return InsertIt;
    }

    // Posiciona o seek onde está apontando que é o nó raiz
    fseek(arquivo, ptr, SEEK_SET);

    // Le o bloco contendo nó raiz
    fread(blocoPtr, TAM_BLOCO, 1, arquivo);
    fclose(arquivo);


    // Observações de acontecimentos no código em geral:
    // Todo lugar que antigamente era a struct node *ptr, agora é blocoBtr->no
    // E todo lugar que era ponteiro para struct de filho, agora é long int


    n = blocoPtr->no.n; // salva em n a quantidade de chaves no nó raiz
    pos = searchPos(key, blocoPtr->no.keys, n);  // Passa a key que quer inserir, todas as chaves presentes no nó e a qtd de chaves no nó
    if (pos < n && key == blocoPtr->no.keys[pos]) // Se a chave a ser inserida ja está presente no nó
        return Duplicate;
    value = ins(blocoPtr->no.p[pos], key, &newKey, &newPtr, &newPtrPos); // Chama recursivamente o metodo ins passando o ponteiro do filho como raiz
    if (value != InsertIt)
        return value;
    /*If keys in node is less than M-1 where M is order of B tree*/
    if (n < M - 1)   // se o nó não estiver cheio
    {
        pos = searchPos(newKey, blocoPtr->no.keys, n);
        /*Shifting the key and pointer right for inserting the new key*/
        for (i=n; i>pos; i--)
        {
            blocoPtr->no.keys[i] = blocoPtr->no.keys[i-1];   // anda com as chaves 1 para direita
            blocoPtr->no.p[i+1] = blocoPtr->no.p[i];         // anda com o ponteiro dos filhos 1 para direita
        }
        /*Key is inserted at exact location*/
        blocoPtr->no.keys[pos] = newKey;
        blocoPtr->no.p[pos+1] = newPtrPos;
        blocoPtr->no.n += 1; /*incrementing the number of keys in node*/

        arquivo = fopen("arquivo.txt", "rb+");

        fseek(arquivo, ptr, SEEK_SET);
        fwrite(blocoPtr, TAM_BLOCO, 1, arquivo);
        fclose(arquivo);
        
        return Success;
    }/*End of if */
    /*If keys in nodes are maximum and position of node to be inserted is last*/
    if (pos == M - 1)
    {
        lastKey = newKey;
        //lastPtr = newPtr;
        lastPtrPos = newPtrPos;
    }
    else /*If keys in node are maximum and position of node to be inserted is not last*/
    {
        lastKey = blocoPtr->no.keys[M-2];  // salva key da posicao M-2
        lastPtrPos = blocoPtr->no.p[M-1];
        for (i=M-2; i>pos; i--)    // SHIFT para inserir key
        {
            blocoPtr->no.keys[i] = blocoPtr->no.keys[i-1];
            blocoPtr->no.p[i+1] = blocoPtr->no.p[i];
        }
        blocoPtr->no.keys[pos] = newKey;
        blocoPtr->no.p[pos+1] = newPtrPos;
    }

    // Encontra mediana e salva em upKey (promovida)
    splitPos = (M - 1)/2;
    (*upKey) = blocoPtr->no.keys[splitPos];

    (*newnode)=malloc(sizeof(struct node));/*Right node after split*/
    blocoPtr->no.n = splitPos; /*No. of keys for left splitted node*/
    (*newnode)->n = M-1-splitPos;/*No. of keys for right splitted node*/ // Se for ordem Par, direita é maior
    for (i=0; i < (*newnode)->n; i++)
    {
        (*newnode)->p[i] = blocoPtr->no.p[i + splitPos + 1]; // Transfere os filhos do blocoPtr para o novo nó
        if(i < (*newnode)->n - 1)
            (*newnode)->keys[i] = blocoPtr->no.keys[i + splitPos + 1]; // Transfere as chaves do blocoPtr para o novo nó
        else
            (*newnode)->keys[i] = lastKey;
        blocoPtr->no.keys[i + splitPos + 1] = 0;
        blocoPtr->no.p[i + splitPos + 1] = 0;
    }
    blocoPtr->no.keys[splitPos] = 0;
    (*newnode)->p[(*newnode)->n] = lastPtrPos;

    // Cria novo Bloco blocoNewNode
    struct Bloco* blocoNewNode = criarBloco();

    // Abre arquivo para leitura e escrita
    arquivo = fopen("arquivo.txt", "rb+");

    // Posiciona o seek onde está o blocoPtr
    fseek(arquivo, ptr, SEEK_SET);
    // Reescreve no arquivo o blocoPtr
    fwrite(blocoPtr, TAM_BLOCO, 1, arquivo);

    // Posiciona seek no final do arquivo
    fseek(arquivo, 0, SEEK_END);

    // Salva o valor da posição final em newNodePos, pois será usado para inserir novo nó posteriormente
    *newNodePos = ftell(arquivo);

    // O nó do novo bloco criado rececebe o novo nó criado anteriormente
    blocoNewNode->no = **newnode;

    // Escreve esse novo bloco no final do arquivo
    fwrite(blocoNewNode, TAM_BLOCO, 1, arquivo);

    // Fecha arquivo
    fclose(arquivo);

    return InsertIt;
}/*End of ins()*/

void display(long int ptr, int blanks)
{
    // Cria FILE arquivo
    FILE *arquivo;
    // Cria Bloco bloco
    struct Bloco* bloco = criarBloco();
    if (ptr != -1)
    {
        int i;
        for(i=1; i<=blanks; i++)
            printf(" ");

        // Abre o arquivo para leitura
        arquivo = fopen("arquivo.txt", "rb");

       // Le o bloco do nó apontado por ptr
        fseek(arquivo, ptr, SEEK_SET);
        fread(bloco, TAM_BLOCO,1,arquivo);

        // Substituir todo lugar que era ptr-> por bloco->no.
        
        for (i=0; i < bloco->no.n; i++)
            printf("%d ",bloco->no.keys[i]);
        printf("\n");

        for (i=0; i <= bloco->no.n; i++)
            display(bloco->no.p[i], blanks+10);
    }/*End of if*/
}/*End of display()*/

void search(int key)
{
    FILE *arquivo;
    int pos, i, n;

    long int ptr;
    struct Bloco *bloco = criarBloco();
    struct Cabecalho* cabecalho = criarCabecalho();

    arquivo = fopen("arquivo.txt", "rb");

    // Le cabecalho
    fseek(arquivo, 0, SEEK_SET);
    fread(cabecalho, TAM_CABECALHO, 1, arquivo);

    // ptr aponta para posicao no arquivo da raiz marcada pelo calendario
    ptr = cabecalho->root;

    printf("Search path:\n");
    while (ptr != -1)
    {
        // Le o bloco do nó apontado por ptr
        fseek(arquivo, ptr, SEEK_SET);
        fread(bloco, TAM_BLOCO,1,arquivo);
        n = bloco->no.n;

        // Exibe todas as chaves do nó
        for (i=0; i < bloco->no.n; i++)
            printf(" %d",bloco->no.keys[i]);
        printf("\n");

        // Verifica qual posicao deveria estar a chave
        pos = searchPos(key, bloco->no.keys, n);

        //Verifica se a chave está no nó atual
        if (pos < n && key == bloco->no.keys[pos])
        {
            printf("Key %d found in position %d of last dispalyed node\n",key,pos+1);
            return;
        }

        // Se a chave não estiver no nó, ptr recebe o filho que poderia conter a chave e retorna no loop, lendo esse filho
        ptr = bloco->no.p[pos]; // ptr recebe posicao no arquivo do filho do caminho para encontrar a chave
    }
    printf("Key %d is not available\n",key);
}/*End of search()*/

// Encontra posicao onde o chave deveria estar
int searchPos(int key, int *key_arr, int n)
{
    int pos=0;
    while (pos < n && key > key_arr[pos])     // Percorre por todas as chaves ate encontrar um espaço onde deveria inserir
        pos++;
    return pos;
}/*End of searchPos()*/

// void DelNode(int key)
// {
//     struct node *uproot;
//     enum KeyStatus value;
//     value = del(root,key);
//     switch (value)
//     {
//     case SearchFailure:
//         printf("\nKey %d is not available\n",key);
//         break;
//     case LessKeys:
//         uproot = root;
//         root = root->p[0];
//         free(uproot);
//         break;
//     }/*End of switch*/
// }/*End of delnode()*/

// enum KeyStatus del(struct node *ptr, int key)
// {
//     int pos, i, pivot, n ,min;
//     int *key_arr;
//     enum KeyStatus value;
//     struct node **p,*lptr,*rptr;

//     if (ptr == NULL)
//         return SearchFailure;
//     /*Assigns values of node*/
//     n=ptr->n;
//     key_arr = ptr->keys;
//     p = ptr->p;
//     min = (M - 1)/2;/*Minimum number of keys*/

//     //Search for key to delete
//     pos = searchPos(key, key_arr, n);
//     // p is a leaf
//     if (p[0] == NULL)
//     {
//         if (pos == n || key < key_arr[pos])
//             return SearchFailure;
//         /*Shift keys and pointers left*/
//         for (i=pos+1; i < n; i++)
//         {
//             key_arr[i-1] = key_arr[i];
//             p[i] = p[i+1];
//         }
//         return --ptr->n >= (ptr==root ? 1 : min) ? Success : LessKeys;
//     }/*End of if */

//     //if found key but p is not a leaf
//     if (pos < n && key == key_arr[pos])
//     {
//         struct node *qp = p[pos], *qp1;
//         int nkey;
//         while(1)
//         {
//             nkey = qp->n;
//             qp1 = qp->p[nkey];
//             if (qp1 == NULL)
//                 break;
//             qp = qp1;
//         }/*End of while*/
//         key_arr[pos] = qp->keys[nkey-1];
//         qp->keys[nkey - 1] = key;
//     }/*End of if */
//     value = del(p[pos], key);
//     if (value != LessKeys)
//         return value;

//     if (pos > 0 && p[pos-1]->n > min)
//     {
//         pivot = pos - 1; /*pivot for left and right node*/
//         lptr = p[pivot];
//         rptr = p[pos];
//         /*Assigns values for right node*/
//         rptr->p[rptr->n + 1] = rptr->p[rptr->n];
//         for (i=rptr->n; i>0; i--)
//         {
//             rptr->keys[i] = rptr->keys[i-1];
//             rptr->p[i] = rptr->p[i-1];
//         }
//         rptr->n++;
//         rptr->keys[0] = key_arr[pivot];
//         rptr->p[0] = lptr->p[lptr->n];
//         key_arr[pivot] = lptr->keys[--lptr->n];
//         return Success;
//     }/*End of if */
// //if (posn > min)
//     if (pos < n && p[pos + 1]->n > min)
//     {
//         pivot = pos; /*pivot for left and right node*/
//         lptr = p[pivot];
//         rptr = p[pivot+1];
//         /*Assigns values for left node*/
//         lptr->keys[lptr->n] = key_arr[pivot];
//         lptr->p[lptr->n + 1] = rptr->p[0];
//         key_arr[pivot] = rptr->keys[0];
//         lptr->n++;
//         rptr->n--;
//         for (i=0; i < rptr->n; i++)
//         {
//             rptr->keys[i] = rptr->keys[i+1];
//             rptr->p[i] = rptr->p[i+1];
//         }/*End of for*/
//         rptr->p[rptr->n] = rptr->p[rptr->n + 1];
//         return Success;
//     }/*End of if */

//     if(pos == n)
//         pivot = pos-1;
//     else
//         pivot = pos;

//     lptr = p[pivot];
//     rptr = p[pivot+1];
//     /*merge right node with left node*/
//     lptr->keys[lptr->n] = key_arr[pivot];
//     lptr->p[lptr->n + 1] = rptr->p[0];
//     for (i=0; i < rptr->n; i++)
//     {
//         lptr->keys[lptr->n + 1 + i] = rptr->keys[i];
//         lptr->p[lptr->n + 2 + i] = rptr->p[i+1];
//     }
//     lptr->n = lptr->n + rptr->n +1;
//     free(rptr); /*Remove right node*/
//     for (i=pos+1; i < n; i++)
//     {
//         key_arr[i-1] = key_arr[i];
//         p[i] = p[i+1];
//     }
//     return --ptr->n >= (ptr == root ? 1 : min) ? Success : LessKeys;
// }/*End of del()*/

void eatline(void) {
  char c;
  printf("");
  while ((c=getchar())!='\n') ;
}

// /* Function to display each key in the tree in sorted order (in-order traversal)
//     @param struct node *ptr, the pointer to the node you are currently working with
//     */
// void inorder(struct node *ptr) {
//     if (ptr) {
//         if (ptr->n >= 1) {
//             inorder(ptr->p[0]);
//             printf("%d ", ptr->keys[0]);
//             inorder(ptr->p[1]);
//             if (ptr->n == 2) {
//                 printf("%d ", ptr->keys[1]);
//                 inorder(ptr->p[2]);
//             }
//         }
//     }
// }

// /* Function that returns the total number of keys in the tree.
//     @param struct node *ptr, the pointer to the node you are currently working with
//     */
// int totalKeys(struct node *ptr) {
//     if (ptr) {
//         int count = 1;
//         if (ptr->n >= 1) {
//             count += totalKeys(ptr->p[0]);
//             count += totalKeys(ptr->p[1]);
//             if (ptr->n == 2) count += totalKeys(ptr->p[2]) + 1;
//         }
//         return count;
//     }
//     return 0;
// }

// /* Function that prints the total number of keys in the tree.
// 	@param struct node *ptr, the pointer to the node you are currently working with
// 	*/
// void printTotal(struct node *ptr) {
// 	printf("%d\n",totalKeys(ptr));
// }

// /* Function that returns the smallest key found in the tree.
//     @param struct node *ptr, the pointer to the node you are currently working with
//     */
// int getMin(struct node *ptr) {
//     if (ptr) {
//         int min;
//         if (ptr->p[0] != NULL) min = getMin(ptr->p[0]);
//         else min = ptr->keys[0];
//         return min;
//     }
//     return 0;
// }

// /* Function that returns the largest key found in the tree.
//     @param struct node *ptr, the pointer to the node you are currently working with
//     */
// int getMax(struct node *ptr) {
//     if (ptr) {
//         int max;
//         if (ptr->n == 1) {
//             if (ptr->p[1] != NULL) max = getMax(ptr->p[1]);
//             else max = ptr->keys[0];
//         }
//         if (ptr->n == 2) {
//             if (ptr->p[2] != NULL) max = getMax(ptr->p[2]);
//             else max = ptr->keys[1];
//         }
//         return max;
//     }
//     return 0;
// }

//  Function that prints the smallest and largest keys found in the tree.
//     @param struct node *ptr, the pointer to the node you are currently working with
    
// void getMinMax(struct node *ptr) {
//     printf("%d %d\n", getMin(ptr), getMax(ptr));
// }

// /* Function that determines the largest number.
// 	@param int, integer to compare.
// 	@param int, integer to compare.
// 	@param int, integer to compare.
// 	*/
// int max(int first, int second, int third) {
// 	int max = first;
// 	if (second > max) max = second;
// 	if (third > max) max = third;
// 	return max;
// }

// /*Function that finds the maximum level in the node and returns it as an integer.
// 	@param struct node *ptr, the node to find the maximum level for.
// 	*/
// int maxLevel(struct node *ptr) {
// 	if (ptr) {
// 		int l = 0, mr = 0, r = 0, max_depth;
// 		if (ptr->p[0] != NULL) l = maxLevel(ptr->p[0]);
// 		if (ptr->p[1] != NULL) mr = maxLevel(ptr->p[1]);
// 		if (ptr->n == 2) {
// 			if (ptr->p[2] != NULL) r = maxLevel(ptr->p[2]);
// 		}
// 		max_depth = max(l, mr, r) + 1;
// 		return max_depth;
// 	}
// 	return 0;
// }

// /*Function that prints the maximum level in the tree.
// 	@param struct node *ptr, the tree to find the maximum level for.
// 	*/
// void printMaxLevel(struct node *ptr) {
// 	int max = maxLevel(ptr) - 1;
// 	if (max == -1) printf("tree is empty\n");
// 	else printf("%d\n", max);
// }


// Funçoes para manipulação de criaArquivo

//Funcao para alocar um novo bloco na memoria.
struct Bloco* criarBloco(){
	struct Bloco *bloco = (struct Bloco*) malloc(sizeof(struct Bloco));
	memset(bloco, 0, TAM_BLOCO);
	return bloco;
}

//Funcao para alocar um cabecalho na memoria
struct Cabecalho* criarCabecalho(){
    struct Cabecalho *cabecalho = (struct Cabecalho*) malloc(sizeof(struct Cabecalho));
    cabecalho->root = -1;
    return cabecalho;
}

//Funcao para criar um novo arquivo.
void criarArquivo(FILE* arquivo) {
	arquivo = fopen("arquivo.txt", "wb");
	if (arquivo == NULL) {
		printf("\nFailed to create file\n");
	}
  else{
    struct Cabecalho *cabecalho = criarCabecalho();
    cabecalho->root = -1;
    fwrite(cabecalho, TAM_CABECALHO, 1, arquivo);
    printf("\nFile successfully created\n");
  }
	fclose(arquivo);
}
