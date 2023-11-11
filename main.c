
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// a struct to keep frequency and binary code representation of a character
typedef struct code {
  unsigned int freq;
  char *binary_code;
} Code;

typedef struct Node {
  unsigned int freq;
  char letter;
  struct Node *left;
  struct Node *right;
} Node;

Node *init_node(char letter, unsigned int freq) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->letter = letter;
  node->freq = freq;
  node->left = NULL;
  node->right = NULL;
  return node;
}

typedef struct Heap {
  int capacity;
  int size;
  Node **array;
} Heap;

Heap *init_heap(int capacity) {
  Heap *heap = (Heap *)malloc(sizeof(Heap));
  heap->capacity = capacity;
  heap->size = 0;
  heap->array = (Node **)malloc(sizeof(Node *) * capacity);
  for (int i = 0; i < heap->capacity; i++) {
    heap->array[i] = NULL;
  }
  return heap;
}

int parent(Heap *heap, int index) {
  int parent_index = (int)((index - 1) / 2);
  return parent_index;
}

void swap(Node **codes, int index, int index2) {
  Node *tmp = codes[index];
  codes[index] = codes[index2];
  codes[index2] = tmp;
}

void upheap(Heap *myHeap, int index) {
  if (index == 0)
    return; // current node is the root
  int parentIndex = parent(myHeap, index);
  Node *parentValue = myHeap->array[parentIndex];
  if (parentValue->freq <= myHeap->array[index]->freq)
    return; // current node’s value is larger than its parent’s
  // else, we need to upheap current value by swapping with the parent
  swap(myHeap->array, index, parentIndex);
  upheap(myHeap, parentIndex);
}

void insert(Heap *myHeap, Node *value) {
  myHeap->array[myHeap->size] = value;
  upheap(myHeap, myHeap->size);
  myHeap->size++;
}

int min_child_index(Heap *heap, int index) {
  if (heap->size - 1 >= index * 2 + 1) { /*not a leaf*/
    if (heap->array[index * 2 + 1]->freq >= heap->array[index * 2 + 2]->freq)
      return index * 2 + 2;
    else
      return index * 2 + 1;
  } else if (heap->size - 1 == (index * 2 + 2))
    return index * 2 + 2;
  else
    return -1;
}

void down_heap(Heap *heap, int index) {
  if (heap->size - 1 < (index * 2 + 2)) {
    if (heap->size - 1 >= (index * 2 + 1)) {
      if (heap->array[index]->freq >= heap->array[index * 2 + 1]->freq)
        swap(heap->array, index, index * 2 + 1);
    }
    return;
  }
  int min_child = min_child_index(heap, index);
  if (heap->array[index]->freq <= heap->array[min_child]->freq)
    return;
  swap(heap->array, index, min_child);
  down_heap(heap, min_child);
}

void delete_min(Heap *heap) {
  swap(heap->array, 0, heap->size - 1);
  heap->size--;
  down_heap(heap, 0);
}

void print_heap(Node *node) {
  if (node == NULL) {
    return;
  }
  Node *left = node->left;
  Node *right = node->right;
  print_heap(left);
  if (left && left->letter != '\0') {
    printf("%c: %d\n", left->letter, left->freq);
  }
  print_heap(right);
  if (right && right->letter != '\0') {
    printf("%c: %d\n", right->letter, right->freq);
  }
}

void build_table(Node *node, Code *codeTable, char *string, int depth) {
  if (node == NULL) {
    return;
  }
  Node *left = node->left;
  Node *right = node->right;
  if (left) {
    char *new_string = (char *)malloc(sizeof(char) * (depth + 2));
    int tracker = 0;
    for (int i = 0; i < depth; i++) {
      new_string[i] = string[i];
    };
    new_string[depth] = '0';
    new_string[depth + 1] = '\0';
    codeTable[left->letter].binary_code = new_string;
    build_table(left, codeTable, new_string, depth + 1);
  }
  if (right) {
    char *new_string = (char *)malloc(sizeof(char) * (depth + 2));
    int tracker = 0;
    for (int i = 0; i < depth; i++) {
      new_string[i] = string[i];
    };
    new_string[depth] = '1';
    new_string[depth + 1] = '\0';
    codeTable[right->letter].binary_code = new_string;
    build_table(right, codeTable, new_string, depth + 1);
  }
}

void write_table(FILE *fp, Node *node, Code *table) {
  if (node == NULL) {
    return;
  }
  Node *left = node->left;
  Node *right = node->right;
  write_table(fp, left, table);
  if (left && left->letter != '\0') {
    fprintf(fp, "%c\t%s\t%d\n", left->letter, table[left->letter].binary_code,
            left->freq);
  }
  write_table(fp, right, table);
  if (right && right->letter != '\0') {
    fprintf(fp, "%c\t%s\t%d\n", right->letter, table[right->letter].binary_code,
            right->freq);
  }
}

char *append_char(char *string, char c) {
  size_t length = strlen(string);
  char *new_string = (char *)malloc(sizeof(char) * (length + 2));
  for (int i = 0; i < length; i++) {
    new_string[i] = string[i];
  };
  new_string[length] = c;
  new_string[length + 1] = '\0';
  // free(string);
  return new_string;
}

char write_decode(char string, Node *node, FILE *fp) {
  if (node == NULL) {
    return '\0';
  }
  if (string == '0') {
    Node *left = node->left;
    if (left->left) {
      char c1 = fgetc(fp);
      return write_decode(c1, node->left, fp);
    } else {
      return left->letter;
    }
  } else if (string == '1') {
    Node *right = node->right;
    if (right->left) {
      char c1 = fgetc(fp);
      return write_decode(c1, node->right, fp);
    } else {
      return right->letter;
    }
  }
  return '\0';
}

int main(int argc, char **argv) {
  // argv[1] will be "encode" or "decode" signifying the mode of the program
  // for encode mode
  // argv[2] will be the path to input text file
  // argv[3] will be the path to output code table file
  // argv[4] will be the path to output encoded text file
  // for decode mode
  // argv[2] will be the path to input code table file
  // argv[3] will be the path to input encoded text file
  // argv[4] will be the path to output decoded text file

  /*----------------ENCODER-----------------------*/
  /*----------------------------------------------*/
  /*To read the input text file, you might want to use a code as follows*/
  char *encode = argv[1];
  int is_encode;
  if (!strcmp(encode, "encode")) {
    is_encode = 1;
  } else if (!strcmp(encode, "decode")) {
    is_encode = 0;
    char *code_table_path = argv[2];
    char *encoded_text_path = argv[3];
    char *decoded_text_path = argv[4];
    FILE *code_table_text = fopen(code_table_path, "r");
    char c;
    int cnt = 0;
    unsigned int freq = 0;
    int factor = 1;
    char letter;
    char *code = (char *)malloc(sizeof(char) * 1);
    Code *codeTable = (Code *)malloc(sizeof(Code) * 256);
    int num = 0;
    // set counters to zero initially
    for (int i = 0; i < 256; i++) {
      codeTable[i].freq = 0;
      codeTable[i].binary_code = "";
    }
    while ((c = fgetc(code_table_text)) != EOF) {
      if (c == '\t') {
        cnt++;
        continue;
      } else if (c == '\n') {
        cnt = 0;
        factor = 1;
        codeTable[letter].freq = freq;
        codeTable[letter].binary_code = code;
        freq = 0;
      } else {
        if (cnt == 0) {
          num++;
          letter = c;
        } else if (cnt == 1) {
          code = append_char(code, c);
        } else if (cnt == 2) {
          freq *= factor;
          freq += c - 48;
          factor *= 10;
        }
      }
    }
    Heap *heap = init_heap(num);
    for (int i = 0; i < 256; i++) {
      if (codeTable[i].freq > 0) {
        Node *node = init_node((char)i, codeTable[i].freq);
        insert(heap, node);
      }
    }
    while (heap->size > 1) {
      Node *min1 = heap->array[0];
      delete_min(heap);
      Node *min2 = heap->array[0];
      delete_min(heap);
      Node *node = init_node('\0', min1->freq + min2->freq);
      node->left = min1;
      node->right = min2;
      insert(heap, node);
    }
    Node *root = heap->array[0];
    FILE *decoded_text = fopen(decoded_text_path, "w");
    FILE *encoded_text = fopen(encoded_text_path, "r");
    while ((c = fgetc(encoded_text)) != EOF) {
      char p = write_decode(c, root, encoded_text);
      fprintf(decoded_text, "%c", p);
    }
    fclose(decoded_text);
    fclose(encoded_text);
    return 1;
  } else {
    printf("Unknown command for encode");
    return -1;
  }
  char *inputTextFilePath = argv[2];
  char *codeTableFilePath = argv[3];
  char *encodeFilePath = argv[4];
  FILE *inputFile = fopen(inputTextFilePath, "r");
  if (inputFile == NULL) {
    printf("Could not open file to read: %s\n", inputTextFilePath);
    return -1;
  }

  // initialize the code table, which will store the frequency of each
  // character, and eventually the binary code. We are allocating a space of 256
  // in the table, and the character's ASCII code will serve as hashing the
  // index for that character.
  Code *codeTable = (Code *)malloc(sizeof(Code) * 256);
  // set counters to zero initially
  for (int i = 0; i < 256; i++) {
    codeTable[i].freq = 0;
    codeTable[i].binary_code = "";
  }

  int totalNumOfCharacters = 0;
  // read the file character by character
  char c;
  while ((c = fgetc(inputFile)) != EOF && c != '\n') {
    // increase the frequency of the character by one
    codeTable[c].freq++;
    // increase the total character count
    totalNumOfCharacters++;
  }
  fclose(inputFile);

  int num_char = 0;
  Heap *heap = init_heap(totalNumOfCharacters);
  for (int i = 0; i < 256; i++) {
    if (codeTable[i].freq > 0) {
      num_char++;
      Node *node = init_node((char)i, codeTable[i].freq);
      insert(heap, node);
    }
  }
  while (heap->size > 1) {
    Node *min1 = heap->array[0];
    delete_min(heap);
    Node *min2 = heap->array[0];
    delete_min(heap);
    Node *node = init_node('\0', min1->freq + min2->freq);
    node->left = min1;
    node->right = min2;
    // print_heap(node);
    insert(heap, node);
  }
  Node *root = heap->array[0];

  heap = init_heap(totalNumOfCharacters);
  for (int i = 0; i < 256; i++) {
    if (codeTable[i].freq > 0) {
      num_char++;
      Node *node = init_node((char)i, codeTable[i].freq);
      insert(heap, node);
    }
  }

  build_table(root, codeTable, "", 0);

  FILE *code_table_fp = fopen(codeTableFilePath, "w");
  if (code_table_fp == NULL) {
    printf("Could not open file to write: %s\n", encodeFilePath);
    return -1;
  }
  for (int i = heap->size - 1; i >= 0; i--) {
    fprintf(code_table_fp, "%c\t%s\t%d\n", heap->array[i]->letter,
            codeTable[heap->array[i]->letter].binary_code,
            heap->array[i]->freq);
  }
  fclose(code_table_fp);
  inputFile = fopen(inputTextFilePath, "r");
  if (inputFile == NULL) {
    printf("Could not open file to read: %s\n", inputTextFilePath);
    return -1;
  }

  FILE *encoded = fopen(encodeFilePath, "w");
  if (encoded == NULL) {
    printf("Could not open file to read: %s\n", encodeFilePath);
    return -1;
  }
  while ((c = fgetc(inputFile)) != EOF && c != '\n') {
    fprintf(encoded, "%s", codeTable[c].binary_code);
  }
  fclose(encoded);

  int totalNumOfCharacters2 = 0;
  fseek(inputFile, 0, SEEK_SET);
  while ((c = fgetc(inputFile)) != EOF && c != '\n') {
    totalNumOfCharacters2++;
  }
  fclose(inputFile);

  // /*----------------------------------------------*/
  // //To print the statistics about the compression, use print statements as
  // follows printf("Original: %d bits\n", uncompressed*8); //assuming that you
  // store the number of characters in variable "uncompressed". *8 is because
  // ASCII table uses 8 bits to represent each character printf("Compressed: %d
  // bits\n", compressed_size); //assuming that you store the number of bits
  // (i.e., 0/1s) of encoded text in variable "compressed_size"
  printf("Compression Ratio: %.2f%%\n",
         (float)totalNumOfCharacters2 / ((float)totalNumOfCharacters * 8) *
             100); // This line will print
  // the compression ration in percentages, up to 2 decimals.

  /*----------------------------------------------*/
  // to write encoded version of the text in 0/1 form into text file, you can
  // use a code similar to fprintf statment above that is suggested for writing
  // code table to the file.

  /*----------------DECODER-----------------------*/
  /*----------------------------------------------*/
  // When decoding, you will need to:
  // 1) read code table: you can use fscanf() function, since the code table
  // file is well structured. Alternatively, you can also use the read
  // statements from above as was used for reading input text file. 2) read
  // encoded text, which is a single line consisting of 0/1 characters: This
  // file is better be read character by character, for which you can use a code
  // similar to getc() code above 3) write the decoded text into file: for that,
  // you can write it into the file using a code similar to fprintf() usages
  // exemplified above.
}
