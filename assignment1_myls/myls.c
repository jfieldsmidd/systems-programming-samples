/*
 * myls.c
 *
 * usage ./myls [-a] [-l] [directory-name]... [file-name]...
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#define DATESTRING_SIZE 13

void printItem (char *path, char *name, int hasOptL);
void printDirOrFile(char *directory_path, int show_hidden_files, int show_file_details, int print_directory_name);

int
main(int argc, char *argv[])
{
  int opt, show_hidden_files, show_file_details;

  /* parses command line args and sets variables for option args
   * optind is set to index of first non-option arg*/
  show_hidden_files = 0;
  show_file_details = 0;
  while ((opt = getopt(argc, argv, "al")) != -1) {
    switch (opt) {
    case 'a':
      show_hidden_files = 1;
      break;
    case 'l':
      show_file_details = 1;
      break;
    default:
      fprintf(stderr, "Usage: %s [-a] [-l] [directory-name]... [file-name]...\n",argv[0]);
      exit(1);
    }
  }

  /* if there are no non-option args, print the working dir */
  if(argv[optind] == NULL){
    printDirOrFile(".", show_hidden_files, show_file_details, 0);
  } else if (argv[optind + 1] == NULL){
    /* if there is 1 non-option arg, print the given dir without the directory name*/
    printDirOrFile(argv[optind], show_hidden_files, show_file_details, 0);
  } else {
    /* iterates through non-option args as directories then prints each entry */
    while(argv[optind]){
      printDirOrFile(argv[optind], show_hidden_files, show_file_details, 1);
      optind ++;
    }
  }
}

//prints out detailed information for -l argument
void printItem (char *path, char *name, int show_hidden_files) {
  struct passwd *pw;
  struct group *gr;
  struct stat info;

  if(stat(path, &info) == -1){ //saves file metadata from stat in struct info
    perror("stat");
    return;
  }

  if (show_hidden_files){
    printf("%s", (S_ISDIR(info.st_mode) ? "d" : "-"));
    printf("%s", ((info.st_mode & S_IRUSR) ? "r" : "-"));
    printf("%s", ((info.st_mode & S_IWUSR) ? "w" : "-"));
    printf("%s", ((info.st_mode & S_IXUSR) ? "x" : "-"));
    printf("%s", ((info.st_mode & S_IRGRP) ? "r" : "-"));
    printf("%s", ((info.st_mode & S_IWGRP) ? "w" : "-"));
    printf("%s", ((info.st_mode & S_IXGRP) ? "x" : "-"));
    printf("%s", ((info.st_mode & S_IROTH) ? "r" : "-"));
    printf("%s", ((info.st_mode & S_IWOTH) ? "w" : "-"));
    printf("%s ", ((info.st_mode & S_IXOTH) ? "x" : "-"));
    printf("%lu ", info.st_nlink);
    if((pw = getpwuid(info.st_uid)) == NULL){
      printf("%d ", info.st_uid);
    }
    else{
      printf("%s ", pw->pw_name);
    }
    if((gr = getgrgid(info.st_gid)) == NULL){
      printf("%d ", info.st_gid);;
    }
    else{
      printf("%s ", gr->gr_name);
    }
    printf("%ld   \t", info.st_size);
    char formattedStringArray[DATESTRING_SIZE];
    strftime(formattedStringArray, DATESTRING_SIZE, "%b %d %H:%M", localtime(&info.st_mtim.tv_sec));
    char *formattedString = formattedStringArray;
    printf("%s ",formattedString);
    printf("%s\n", name);
  } else{
    printf("%s\n", name);
  }
}

void printDirOrFile(char *directory_path, int show_hidden_files, int show_file_details, int print_directory_name) {
  DIR *stream;
  struct dirent *entry;
  char path[PATH_MAX];

  /* handles filenames given as args */
  struct stat path_stat;
  if(stat(directory_path, &path_stat) == -1){ //saves file metadata from stat in struct info
    perror("stat");
    return;
  }
  if (S_ISREG(path_stat.st_mode)) {
    printItem(directory_path, directory_path, show_file_details);
    printf("\n");
    return;
  }

  if((stream = opendir(directory_path)) == NULL){
    perror("opendir");
    return;
  }

  strcpy(path, directory_path);//copies the directory name into path
  size_t directory_length = strlen(directory_path);
  path[directory_length] = '/';

  if(print_directory_name){
    printf("%s:\n", directory_path);
  }

  while((entry = readdir(stream))){ //iterate through entries
    char *name;
    name = entry->d_name;

    strcpy(path + directory_length + 1, name);//adds the entry name to path

    if(show_hidden_files == 0 && name[0] == '.'){
      continue;
    }
    printItem(path, name, show_file_details);

  }
  printf("\n");
  closedir(stream);
}
