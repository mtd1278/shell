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
#include <fcntl.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 32     

void fork_and_exec_cmd(char * argv[], int token_count)
{
  pid_t pid = fork();

  if (pid == -1)
  {
    char error_message[30] = "An error has occurred\n";                
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  {
    if (argv[0] != NULL)
    {
      int i;
      for( i=1; i<token_count; i++ )
      {
        if( strcmp(argv[i], ">") == 0)
        {
            if (argv[i+1] == NULL || argv[i+2] != NULL)
            {
              char error_message[30] = "An error has occurred\n";              
              write(STDERR_FILENO, error_message, strlen(error_message));
              return;
            }
            int fd = open( argv[i+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR ); // read/write, create file, usr read/writepermission bit 
            if( fd < 0 )
            {
                perror( "Can't open output file." );
                exit( 0 );                    
            }
            dup2( fd, 1 );
            close( fd );
            
            // Trim off the > output part of the command
            argv[i] = NULL;
            break;
        }
      } 
    }
    else
    {
      char error_message[30] = "An error has occurred\n";              
      write(STDERR_FILENO, error_message, strlen(error_message));
    }
    int i = execvp(argv[0], argv);                                       
    if( i == -1) 
    {
      char error_message[30] = "An error has occurred\n";              
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

void process_command_string(char * command_string)
{
    char *token[MAX_NUM_ARGUMENTS];  

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
        /*if( token_count != 0 && strlen( token[token_count] ) == 0 )
        {
          token[token_count] = NULL;
        } 
        if ( strlen( token[0] ) == 0) 
        {
          token[token_count] = NULL;
        }
        */
        if ( strlen( token[token_count] ) == 0) 
        {
          token[token_count] = NULL;
        }
        else
        {
          token_count++;
        }
        
      }

      if (token[0] != NULL)
      {
        if ((strcmp(token[0], "exit") != 0) && (strcmp(token[0], "cd") !=0 ))
        { 
          fork_and_exec_cmd(token, token_count); // can use execvp
        }
        else if ((strcmp(token[0], "exit") == 0 && token[1] == NULL))
        {
          exit(0);
        }
        else if (strcmp(token[0], "cd") == 0 && token[1] != NULL && token[2] == NULL)                         
        {
          if (chdir(token[1]) == -1)
          {
            char error_message[30] = "An error has occurred\n";              
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
          }
        }
        else
        {
          char error_message[30] = "An error has occurred\n";              
          write(STDERR_FILENO, error_message, strlen(error_message));
          return;
        }
      }
      else
      {
        return;
      }
      free(head_ptr);
}


int main( int argc, char * argv[] )
{
  char * command_string = (char*) malloc( MAX_COMMAND_SIZE ); 
  
  if (argc == 2)  
  {
    FILE* file = fopen(argv[1], "r");   
    char *buffer= (char*)malloc(MAX_COMMAND_SIZE);
    if (file == NULL)
    {
      char error_message[30] = "An error has occurred\n";             
      write(STDERR_FILENO, error_message, strlen(error_message));
      exit(1);
    }
    size_t n = 80;
    while(fgets(buffer, n, file) != 0)
    {
      process_command_string(buffer);
    }
  fclose(file);
  }
  else if (argc == 1)   
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
      process_command_string(command_string);
    }
    free(command_string);
  }
  else if (argc > 2)
  {
    char error_message[30] = "An error has occurred\n";             
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }
  return 0;
}
