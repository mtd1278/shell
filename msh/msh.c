// The MIT License (MIT)
// 
// Copyright (c) 2024 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <dirent.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 32     

void fork_and_exec_cmd(char * argv[])
{
  pid_t pid = fork();

  if (pid == -1)
  {
    char error_message[30] = "fork error has occurred\n";                // TODO 
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    int i = execv(argv[0], argv);                                         // ls works with /bin/ls
    if( i == -1) 
    {
      char error_message[30] = "execv error has occurred\n";              // TODO
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
    fflush(NULL);
    exit(EXIT_SUCCESS);
  }
  else
  {
    int status;
    waitpid(pid, &status, 0 );
    fflush(NULL);
  }

}

int main( int argc, char * argv[] )
{
  char * command_string = (char*) malloc( MAX_COMMAND_SIZE ); // point to command string 


  if (argc > 1)
  {
    // batch 
    FILE* file = fopen(argv[1], "r");

    fclose(file);
  }
  else
  {                   
    
  while( 1 )
  {
    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the command line.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something.
    while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS]; // array of tokens 

    int token_count = 0;                                 
                                                          
    // Pointer to point to the token
    // parsed by strsep
    char *argument_pointer;                                         
                                                          
    char *working_string  = strdup( command_string );     // strdup reutns a pointer to a duplicated of command string, terminated by null     

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    
    char *head_ptr = working_string;  // head pointer to duplicatd string 
    
    // Tokenize the input with whitespace used as the delimiter
    // argument pointer points to tokenized duplicated string 
    // while token not null and less than max args
    while ( ( (argument_pointer = strsep(&working_string, WHITESPACE ) ) != NULL) && (token_count<MAX_NUM_ARGUMENTS))                                       
    {
      token[token_count] = strndup( argument_pointer, MAX_COMMAND_SIZE );  // duplicate argument pointer to token 
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }


    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    /*
    int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      printf("token[%d] = %s\n", token_index, token[token_index] );  
    }
    */

    // if not exit or cd 
    if ((strcmp(token[0], "exit") != 0) && (strcmp(token[0], "cd") !=0 ) && token[0] != NULL)
    {
      char path0[50] = "/bin/";
      char path1[50] = "/usr/bin/";
      char path2[50] = "/usr/local/bin/";
      char path3[50] = "./";
      char *test[MAX_NUM_ARGUMENTS];
      printf("token [0] in non builtin: %s\n", token[0]);
      test[0]= strcat(path0, token[0]);
      if (access(test[0], X_OK) == 0)
      {
        printf("hi\n");
        token[0] = test[0];
        printf("token [0] in path 0: %s\n", token[0]);
        fork_and_exec_cmd(token); 
      }
      else
      {
        test[0] = strcat(path1, token[0]);
        if (access(test[0], X_OK) == 0)
        {
          printf("hi 1\n");
          token[0] = test[0];
          fork_and_exec_cmd(token); 
        }
        else
        {
          test[0] = strcat(path2, token[0]);
          if (access(test[2], X_OK) == 0)
          {
            printf("hi 2\n");
            token[0] = test[0];
            fork_and_exec_cmd(token); 
          }
          else
          {
            test[0] = strcat(path3, token[0]);
            if (access(test[0], X_OK) == 0)
            {
              printf("hi3 \n");
              token[0] = test[0];
              fork_and_exec_cmd(token); 
            }
            else
            {
              char error_message[50] = "pass to fork&cmd error has occurred\n";              // TODO
              write(STDERR_FILENO, error_message, strlen(error_message));
            }
          }
        }
      }
    }
    else
    {
      if (token[0] != NULL)
      {
        if (strcmp(token[0], "exit") == 0 && token[1] == NULL)
        {
          exit(0);
        }
        else if (strcmp(token[0], "cd") == 0 && token[1] != NULL && token[2] == NULL)                         
        {
          if (chdir(token[1]) == -1)
          {
            char error_message[30] = "An error has occurred\n";              
            write(STDERR_FILENO, error_message, strlen(error_message));
          }
        }
        else
        {
          char error_message[30] = "An error has occurred\n";             
          write(STDERR_FILENO, error_message, strlen(error_message));
        }
      }
    }
    free(head_ptr);
  }
  free(command_string);
  return 0;
  }
}
