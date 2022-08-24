#include "allocation.h"
#include "inode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BLOCKSIZE 4096

static size_t id = 0; // global variabel brukes for id til create_file og create_dir

struct inode* create_file( struct inode* parent, char* name, char readonly, int size_in_bytes )
{
  
  if(find_inode_by_name(parent, name) == NULL){
    struct inode *new_file = malloc(sizeof(struct inode));
    if(new_file == NULL){
      fprintf(stderr, "malloc failed" );
      return NULL;
    }


    new_file->num_entries = (size_in_bytes + BLOCKSIZE - 1) / BLOCKSIZE;
    new_file->entries = malloc(sizeof(size_t)*(new_file->num_entries + 1));
    if(!new_file->entries){
      fprintf(stderr, "malloc failed" );
      return NULL;
    }

    for(int i = 0 ; i < new_file->num_entries; i++){

        new_file->entries[i] = (size_t) allocate_block();

    }

    new_file->id = id;
    id++;

    new_file->name = strdup(name);
    new_file->is_directory = 0;
    new_file->is_readonly = readonly;
    new_file->filesize = size_in_bytes;

      parent->entries = (size_t*) realloc(parent->entries,(sizeof(int)*(parent->num_entries + 1)));
      if(!parent->entries){
        fprintf(stderr, "realloc failed" );
        return NULL;
      }
      parent->num_entries++;
      parent->entries[parent->num_entries-1] = (size_t) new_file;

    return new_file;
  }
else{//else det er et annen inode(fil eller dir) med samma navn
    return NULL;
  }
}

struct inode* create_dir( struct inode* parent, char* name )
{

  if(parent == NULL){
    struct inode *new_dir = malloc(sizeof(struct inode) + 1);

    if(new_dir == NULL){
      fprintf(stderr, "malloc failed" );
      return NULL;
    }
    new_dir->id = id;
    id++;
    
    new_dir->name = strdup(name);
    new_dir->is_directory = 1;
    new_dir->is_readonly = 0;
    new_dir->filesize = 0;

    new_dir->num_entries = 0;//siden det er root dir og har ikke noe entries ennaa
    new_dir->entries = NULL;

    return new_dir;
  }else{
    if(find_inode_by_name(parent, name) == NULL){

      struct inode *new_dir = malloc(sizeof(struct inode) + 1);
      if(!new_dir){
        fprintf(stderr, "malloc failed" );
        return NULL;
      }

      new_dir->id = id;
      id++;
      new_dir->name = strdup(name);
      new_dir->is_directory = 1;
      new_dir->is_readonly = 0;
      new_dir->filesize = 0;


        //greit aa alokere mer enn man trenger..?
        parent->entries = realloc(parent->entries,sizeof(size_t)*(parent->num_entries + 1));//aloker mer enn noed, fearre feil
        parent->num_entries++;
        parent->entries[parent->num_entries-1] = (size_t) new_dir;
      return new_dir;
    }
    return NULL;
  }
}


struct inode* create_inode(FILE* superblock_fil)
{//hjelp funksjon for aa lage inoder rekursivt - brukes i load_inodes

  int id;
  struct inode *inode = malloc(sizeof(struct inode));

  inode->id = id;
  int read_fil;
  read_fil = fread(&inode->id, sizeof(int), 1 , superblock_fil);

  int len;
  read_fil = fread(&len, sizeof(int), 1 , superblock_fil);

  char *name = malloc(len*sizeof(char));
  inode->name = name;

  read_fil = fread(inode->name, sizeof(char), len, superblock_fil);

  read_fil = fread(&inode->is_directory, sizeof(char), 1, superblock_fil);

  read_fil = fread(&inode->is_readonly, sizeof(char), 1, superblock_fil);

  read_fil = fread(&inode->filesize, sizeof(int), 1, superblock_fil);

  read_fil = fread(&inode->num_entries, sizeof(int), 1, superblock_fil);

  // printe inoder
  // fprintf(stderr, "ID: %d\n", inode->id);
  // fprintf(stderr, "Name: %s\n", inode->name);
  // fprintf(stderr, "Is_directory: %d\n", inode->is_directory);
  // fprintf(stderr, "Is_readonly: %d\n", inode->is_readonly);
  // fprintf(stderr, "Filesize: %d\n", inode->filesize);
  // fprintf(stderr, "Num_entries: %d\n", inode->num_entries);
  // printf("\n" );

  size_t *antall_entr = malloc(sizeof(size_t)*inode->num_entries);
  inode->entries = antall_entr;

  read_fil = fread(inode->entries, sizeof(size_t), inode->num_entries, superblock_fil);

  if(inode->is_directory == 1){
    for(int i = 0; i<inode->num_entries;i++){
      inode->entries[i] = (size_t) create_inode(superblock_fil);
    }
  }
  return inode;
}


struct inode* find_inode_by_name( struct inode* parent, char* name )
{
  if(parent->is_directory){
    for(int i = 0; i < parent->num_entries;i++){
      struct inode* child = (struct inode*)parent->entries[i];
      if(strcmp(child->name,name) == 0){
        return child;
      }
    }
  }
  return NULL;
}


struct inode* load_inodes()
{
    FILE *fptr;
    fptr = fopen("superblock","rb");
    if(fptr == NULL)
       {
          printf("Error!");
          fclose(fptr);
          exit(1);
       }

    struct inode* inode_root = create_inode(fptr);//hjelp funksjon

    fclose(fptr);

    return inode_root;
}

void fs_shutdown( struct inode* inode )
{
  if (inode == NULL) {
        return;
    }

    if (inode -> is_directory) {
        for (int i = 0; i < inode -> num_entries; i++) {
            struct inode* child = (struct inode*)inode -> entries[i];
            fs_shutdown(child);
        }
    }

    //free alt som fikk allokert
    free(inode -> name);
    free(inode -> entries);
    free(inode);

}

/* This static variable is used to change the indentation while debug_fs
 * is walking through the tree of inodes and prints information.
 */
static int indent = 0;

void debug_fs( struct inode* node )
{
    if( node == NULL ) return;
    for( int i=0; i<indent; i++ )
        printf("  ");
    if( node->is_directory )
    {
        printf("%s (id %d)\n", node->name, node->id );
        indent++;
        for( int i=0; i<node->num_entries; i++ )
        {
            struct inode* child = (struct inode*)node->entries[i];
            debug_fs( child );
        }
        indent--;
    }
    else
    {
        printf("%s (id %d size %db blocks ", node->name, node->id, node->filesize );
        for( int i=0; i<node->num_entries; i++ )
        {
            printf("%d ", (int)node->entries[i]);
        }
        printf(")\n");
    }
}
