//constraint all functions used must have a signature in the header file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
int MAX_NAME = 100;
int MAX_CMD = 100;
int MAX_LINE=100;
#define MAX_FUNC 1000//constraint
struct cnode *headc = NULL;
struct hnode *headh = NULL;
struct fnode
{
    char *name;//signature
    char *just_name;
    struct fnode *next;
    int self;// if there exist a self recursion
    int visited;// to check if any function is not defined inside any .c file
};
struct hnode
{
    char *name;
    struct fnode **dep;//an array of functions inside the header
    int dep_count;
    struct hnode *next;
};
struct cnode
{
    char *name;
    struct cnode *next;
    struct hnode **dep; // array of headers inside .c files
    int dep_count;
};

// Function signatures
int is_ignored_function(char *name);//Checks whether a detected name is a keyword/function you want to ignore, like if, for, return, or printf
int count_braces_line(char *buffer);//Counts { as +1 and } as -1 in one line.
void write_dot_edge(FILE *fp, char *from, char *to);//Writes one quoted DOT graph edge from one node to another.
void write_dot_node(FILE *fp, char *name, char *fillcolor);//Writes one colored DOT graph node.
void write_unused_headers(FILE *fp, struct hnode **visited, int vcount);//Marks header files that were never included by any .c file.
void write_undefined_functions(FILE *fp);//Marks declared functions that were not found as definitions in any .c file.
char *getname(char *buffer);//Finds a function call in a line by locating ( and reading the function name before it.
void main_processing(FILE *dep);//Finds main() in .c files and writes graph edges from main to functions it calls.
void insertAtFrontC(char *name, int noofhfiles);//Creates a new .c file node and inserts it at the front of the C-file linked list.
void insertfunc(struct fnode **func,struct fnode *ins);//Adds a called-function dependency to a function’s linked list.
void checker(struct fnode **func);//Checks the current function body in extra to find calls to declared header functions.
void mark_definitions();//Marks which header-declared functions are actually defined in .c files.
int process(char *name,char *sig,char *justname);//Checks whether a line looks like the definition of a specific function signature.
void fdeplinker();//Finds function definitions in .c files, extracts their bodies, and links their function-call dependencies.
void finitializer(struct hnode *header);//Reads function declarations from one header file and stores them in that header node.
void insertAtFrontH(char *name);//Creates a new header node, initializes its functions, and inserts it at the front of the header list.
void cinitializer(int noofhfiles);//Finds all .c files in the folder and creates C-file nodes for them.
int hinitializer();//Finds all .h files in the folder, creates header nodes, and returns the number of headers.
void chlinker();//Links each .c file node to the .h files it includes.
void makefile();//Generates a Makefile based on discovered .c and .h dependencies.
void print_structure();//Prints the internal C/header/function dependency structure for debugging.
void dependency_graph(int n);//Writes the complete dep1.dot dependency graph.

int is_ignored_function(char *name)
{
    return strcmp(name, "if") == 0 || strcmp(name, "while") == 0 || strcmp(name, "for") == 0 || strcmp(name, "switch") == 0 || strcmp(name, "return") == 0 || strcmp(name, "sizeof") == 0 || strcmp(name, "printf") == 0 || strlen(name) == 0;
}

int count_braces_line(char *buffer)
{
    int brace_count = 0;
    for(int i=0; buffer[i]!='\0'; i++)
    {
        if(buffer[i]=='{')
            brace_count++;
        if(buffer[i]=='}')
            brace_count--;
    }
    return brace_count;
}

void write_dot_edge(FILE *fp, char *from, char *to)
{
    fprintf(fp,"\t\"%s\"->\"%s\";\n",from,to);
}

void write_dot_node(FILE *fp, char *name, char *fillcolor)
{
    fprintf(fp,"\t \"%s\" [style=filled, fillcolor=%s];\n",name,fillcolor);
}

char *getname(char *buffer)
{
    char *ptr=buffer;
    while((ptr=strstr(ptr,"("))!=NULL)
    {
        //avoid reading before buffer if '(' is the first character.
        if(ptr == buffer)
        {
            ptr++;
            continue;
        }
        char *start=ptr-1;

        //skip spaces
        while(start>=buffer && (*start==' ' || *start=='\t'))
            start--;

        char *end=start;

        //get function name
        while(start>=buffer && (isalnum(*start) || *start=='_'))
            start--;
        
        start++;

        int len=end-start+1;
        if(len<=0)
        {
            ptr++;
            continue;
        }
        char *name = malloc(len + 1);
        strncpy(name, start, len);
        name[len] = '\0';
        // ignore keywords
        if (is_ignored_function(name))
        {
            free(name);
            ptr++; 
            continue;
        }
        return name;
    }
    return NULL;

}
void main_processing(FILE *dep)
{
    struct cnode *cptr=headc;
    while(cptr!=NULL)
    {
        FILE *fp=fopen(cptr->name,"r");
        if (fp == NULL)
        {
            perror("fopen failed");
            cptr = cptr->next;
            continue;
        }
        char buffer[MAX_LINE];
        int flag=0;//in main
        
        int brace_count=0;
        while(fgets(buffer,MAX_LINE,fp)!=NULL)
        {
            if(flag==0)
            {
                
                char *ptr=strstr(buffer,"main");
                
                if(ptr!=NULL)
                {
                    
                    ptr+=4;//move after main
                    
                    
                    //skip spaces
                    while(*ptr==' ' || *ptr=='\t')
                        ptr++;

                    if(*ptr=='(')
                    {
                        flag=1;//valid main
                        //printf("found main in %s\n",cptr->name);

                        
                        //printf("printing main dependency in dependency for %s\n",cptr->name);
                        char main_name[MAX_NAME + 20];
                        sprintf(main_name,"main_of_%s",cptr->name);
                        //use quoted DOT edge so filenames like file2.c are valid.
                        write_dot_edge(dep,cptr->name,main_name);
                        
                        // count braces on same line as main
                        brace_count += count_braces_line(buffer);
                        break;

                    }
                

                    
                }
                

            }
        }
        if(flag==1)
        {
            while(fgets(buffer,MAX_LINE,fp)!=NULL)
            {
                char *temp = buffer;
                char *funcyy;

                while(*temp!='\0' && (funcyy = getname(temp)) != NULL)
                {
                    char main_name[MAX_NAME + 20];
                    sprintf(main_name,"main_of_%s",cptr->name);
                    write_dot_edge(dep,main_name,funcyy);

                    //move ahead in the original buffer to avoid finding the same call forever.
                    char *next = strstr(temp, funcyy);
                    if(next == NULL)
                        temp++;
                    else
                        temp = next + strlen(funcyy);

                    free(funcyy);
                }
                brace_count += count_braces_line(buffer);
                if(brace_count==0)
                    break;
                
                
            }
        }
        fclose(fp);
        cptr = cptr->next;
    }
     
}

void insertAtFrontC(char *name, int noofhfiles)
{
    struct cnode *new = malloc(sizeof(struct cnode));
    new->name = strdup(name);
    // new->next=NULL;
    // constraint contains max noofhfiles many headers
    new->dep = malloc(sizeof(struct hnode *) * noofhfiles);
    // insert at front
    new->next = headc;
    headc = new;
    new->dep_count = 0;
}
void insertfunc(struct fnode **func,struct fnode *ins)
{
    struct fnode *new=malloc(sizeof(struct fnode));

    new->name = strdup(ins->name);
    new->just_name = strdup(ins->just_name);
    new->self = ins->self;
    new->visited=ins->visited;

    new->next=(*func)->next;
    (*func)->next=new;

}
//constraint - not more than 1 c file can have declaration of a particular function
//constraint func - if there is nested block inside function then it should } should not be on a seperate line
void checker(struct fnode **func)
{
    struct hnode *hptr=headh;
    while(hptr!=NULL)
    {
        for(int i=0;i<hptr->dep_count;i++)
        {
            char cmd[MAX_CMD];
            sprintf(cmd,"awk '/%s[ \t]*\\(/ {lines++} END{print lines}' %s",hptr->dep[i]->just_name,"extra");
            //printf("CMD: %s\n", cmd);
            FILE *fp = popen(cmd,"r");
            char buffer[MAX_LINE];
            while(fgets(buffer,MAX_LINE,fp)!=NULL)
            {
                int n=atoi(buffer);
                if(n!=0)
                {
                    //self recursion check
                    if(strcmp((*func)->just_name, hptr->dep[i]->just_name)==0){
                        (*func)->self=1;
                        break;
                    }
                    insertfunc(func,hptr->dep[i]);
                }
            }
            pclose(fp);
        }
        hptr=hptr->next;
    }
}
//whatever functions are defined in .c files marks those as visited (a global scan) 
void mark_definitions()
{
    struct cnode *cptr=headc;
    while(cptr!=NULL)
    {
        FILE *fp=fopen(cptr->name,"r");
        if(fp == NULL){
            perror("fopen failed");
            return;
        }
        for(int i=0;i<cptr->dep_count;i++)
        {
            for(int j=0;j<cptr->dep[i]->dep_count;j++)
            {
                rewind(fp);//brings it back to start of file
                char name[MAX_LINE];
                char cmp[MAX_NAME];
                strcpy(cmp,cptr->dep[i]->dep[j]->name);
                int len = strlen(cmp);
                //removing semicolon
                if (len > 0 && cmp[len - 1] == ';')
                    cmp[len - 1] = '\0';
                while(fgets(name,MAX_LINE,fp)!=NULL)
                {
                    if(strstr(name,cmp))//constraint - function definition should be exactly same as one given in header
                    {
                        cptr->dep[i]->dep[j]->visited = 1;
                        break;
                    }
                }

            }
        }
        fclose(fp);
        cptr = cptr->next;
    }
}
//removing constraint that function defn in .h should be same as func defn in .c
//function defn format- return_type func_name(type a,...)
//constraint - can't add initialization and function call on same line
int process(char *name,char *sig,char *justname)
{
    char ret1[MAX_LINE];
    strcpy(ret1,sig);

    char ret[MAX_LINE];
    strcpy(ret,strtok(ret1," "));//stores return type
    
    strcat(ret," ");

    char j2[MAX_LINE];
    strcpy(j2,justname);

    strcat(j2,"(");

    char jcopy[MAX_LINE];//creating copy so no other string gets modified
    strcpy(jcopy, justname);
    strcat(jcopy, " ");

    if(strstr(name,ret)==NULL || (strstr(name,j2)==NULL && strstr(name,jcopy)==NULL))
        return 0;

    return 1;
    
        
}

//reads function declarations and links functions - constraint function should be declared with same parameters in .c file as .h file
void fdeplinker()
{
    struct cnode *cptr=headc;
    while(cptr!=NULL)
    {
        FILE *fp=fopen(cptr->name,"r");
        if(fp == NULL){
            perror("fopen failed");
            return;
        }
        for(int i=0;i<cptr->dep_count;i++)
        {
            
            for(int j=0;j<cptr->dep[i]->dep_count;j++)
            {
                rewind(fp);//brings it back to start of file
                char name[MAX_LINE];
                char cmp[MAX_NAME];
                char cmp2[MAX_LINE];
                strcpy(cmp,cptr->dep[i]->dep[j]->name);
                strcpy(cmp2,cptr->dep[i]->dep[j]->just_name);
                int len = strlen(cmp);
                //removing semicolon
                if (len > 0 && cmp[len - 1] == ';')
                    cmp[len - 1] = '\0';

                
                while(fgets(name,MAX_LINE,fp)!=NULL)
                {
                    if(process(name,cmp,cmp2))//constraint - function definition should be exactly same as one given in header
                    {
                        //cptr->dep[i]->dep[j]->visited = 1;(job of mark definitions)
                        //moves to opening {
                        while(fgets(name,MAX_LINE,fp) && !strchr(name,'{'));
                        
                        //constraint - the semicolon ending the function should be on a seperate line
                        FILE *fp2=fopen("extra","w");
                        int inbrace=0;
                        int outbrace=0;
                        do// constraint ending } should be on seperate line - removed this constraint
                        {
                            //printing function code into extra
                            fprintf(fp2,"%s",name);
                            for(size_t y=0;y<strlen(name);y++)
                            {
                                if(name[y]=='{')
                                    inbrace++;
                                if(name[y]=='}')
                                    outbrace++;
                            }
                            
                            
                                                    
                        }
                        while(fgets(name,MAX_LINE,fp) != NULL && inbrace!=outbrace);//all braces are matched then inbrace = outbrace
                        
                        if(outbrace>inbrace)
                            printf("syntax error in %s\n",cptr->dep[i]->dep[j]->name);
                        fclose(fp2);
                        
                        checker(&(cptr->dep[i]->dep[j]));
                        break;


                    }
                }
                
            }
        }
        fclose(fp);
        cptr=cptr->next;
    }
}




//to read function names and store inside array
void finitializer(struct hnode *header)
{
    
    char comm[MAX_CMD];
    char esc[MAX_CMD];
    sprintf(esc,"/^[ \t]*[a-zA-Z_][a-zA-Z0-9_]*[ \t]+[a-zA-Z_][a-zA-Z0-9_]*[ \t]*\\(/"); //function signature format
    sprintf(comm,"awk -F ' ' ' %s{print $0}' %s",esc,header->name);
    FILE *fp=popen(comm,"r");
    char bufferr[MAX_NAME];
    int o=0;
    while(fgets(bufferr,MAX_NAME,fp)!=NULL)
    {
        // remove newline
        int len = strlen(bufferr);
        if (len > 0 && bufferr[len - 1] == '\n')
            bufferr[len - 1] = '\0';

        // allocate fnode
        header->dep[o] = malloc(sizeof(struct fnode));
        header->dep[o]->name=strdup(bufferr);
        header->dep[o]->next=NULL;
        header->dep[o]->visited=0;
        header->dep[o]->self = 0;
        o++;
    }
    pclose(fp);

    sprintf(esc,"/^[ \t]*[a-zA-Z_][a-zA-Z0-9_]*[ \t]+[a-zA-Z_][a-zA-Z0-9_]*[ \t]*\\(/"); //function signature format
    sprintf(comm,"awk -F ' ' ' %s{print $2}' %s",esc,header->name);
    FILE *fp2=popen(comm,"r");
    char buffer[MAX_NAME];
    int i=0;
    while(fgets(buffer,MAX_NAME,fp2)!=NULL)
    {
        // remove newline
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';

        // allocate fnode
        header->dep[i]->just_name=strdup(buffer);
        i++;
    }
    pclose(fp2);
}
//function signature - format to be void/int/char... func_name () - constraint space btw func_name ()
void insertAtFrontH(char *name)
{

    struct hnode *new = malloc(sizeof(struct hnode));
    new->name = strdup(name);
    new->next=NULL;
    //initializing with size of array
    char com[MAX_CMD];
    char esc[MAX_CMD];
    sprintf(esc,"/^[ \t]*[a-zA-Z_][a-zA-Z0-9_]*[ \t]+[a-zA-Z_][a-zA-Z0-9_]*[ \t]*\\(/"); //function signature format
    sprintf(com,"awk '%s {count++} END{print count}' %s",esc,new->name);
    FILE *fp=popen(com,"r");
    char buffer[MAX_NAME];
    int nooffunc=0;
    if(fgets(buffer,MAX_NAME,fp)!=NULL)//we got no of functions
    {
        nooffunc=atoi(buffer);
        //printf("%d",nooffunc);
    }

    new->dep = (malloc)(sizeof(struct fnode*)*nooffunc);
    new->dep_count=nooffunc;
    finitializer(new);
    pclose(fp);
    //insert at front
    new->next = headh;
    headh = new;
}
void cinitializer(int noofhfiles)
{
    FILE *fp = popen("ls | grep .c$", "r");
    if (fp == NULL)
        perror("Error in popen in initializer");
    char name[MAX_NAME];
    int flag = 0;
    while (fgets(name, MAX_NAME, fp) != NULL)
    {
        flag = 1;
        // if last char is newline & not blank
        if (strlen(name) > 0 && name[strlen(name) - 1] == '\n')
            name[strlen(name) - 1] = '\0';
        insertAtFrontC(name, noofhfiles);
    }
    if (flag == 0)
        perror("No c file found in initializer");
    pclose(fp);
}
int hinitializer()
{
    FILE *fp = popen("ls | grep .h$", "r");
    if (fp == NULL)
        perror("Error in popen in initializer");
    char name[1000];
    int flag = 0;
    int noofhflies = 0;
    while (fgets(name, MAX_NAME, fp) != NULL)
    {
        noofhflies++;
        flag = 1;
        // if last char is newline & not blank
        if (strlen(name) > 0 && name[strlen(name) - 1] == '\n')
            name[strlen(name) - 1] = '\0';
        insertAtFrontH(name);
    }

    if (flag == 0)
        perror("No c file found in initializer");
    pclose(fp);
    return noofhflies;
}
// reads c file finds all .h #includes and links the dependencies
void chlinker()
{
    struct cnode *ptr = headc;
    while (ptr != NULL)
    {
        char cmd[MAX_CMD];
        char apo = '"';
        sprintf(cmd, "awk -F '%c' '/#include %c/ {print$2}' %s ", apo, apo, ptr->name);
        FILE *fp = popen(cmd, "r");
        char name[MAX_NAME];
        int htracker = 0;
        while (fgets(name, MAX_NAME, fp) != NULL)
        {
            // if last char is newline & not blank
            if (strlen(name) > 0 && name[strlen(name) - 1] == '\n')
                name[strlen(name) - 1] = '\0';
            struct hnode *hptr = headh;
            while (hptr != NULL)
            {
                if (strcmp(name, hptr->name) == 0)
                {
                    ptr->dep[htracker++] = hptr;
                }
                hptr = hptr->next;
            }
        }
        ptr->dep_count = htracker;
        ptr = ptr->next;
        pclose(fp);
    }
}
void makefile()
{
    FILE *fp = fopen("Makefile", "w");
    struct cnode *ptr = headc;

    while (ptr != NULL)
    {
        char oname[MAX_NAME];
        strcpy(oname, ptr->name);
        //.c->.o
        oname[strlen(oname) - 1] = 'o';
        fprintf(fp, "%s: %s", oname, ptr->name);
        for (int i = 0; i < ptr->dep_count; i++)
        {
            fprintf(fp, " %s", ptr->dep[i]->name);
            char cname[MAX_NAME];
            strcpy(cname, ptr->dep[i]->name);
            //.h->.c
            cname[strlen(cname) - 1] = 'c';
            if (strcmp(cname, ptr->name) != 0) // only one for ptr->name.c
                fprintf(fp, " %s", cname);
        }
        fprintf(fp, "\n\t");
        fprintf(fp, "gcc -c %s\n", ptr->name);
        ptr = ptr->next;
    }

    // checking if main is there to link object file to a.out
    ptr = headc;
    while (ptr != NULL)
    {
        char cm[1000];
        sprintf(cm, "grep 'int main()' %s", ptr->name);
        FILE *fp1 = popen(cm, "r");
        // checking main found
        char buffer[1000];
        // if there is main in the .c file create cmg (file1: file1.h'.o'....) gcc file.o file1.o...
        if (fgets(buffer, sizeof(buffer), fp1) != NULL) // main found
        {
            char without[MAX_NAME];
            strcpy(without, ptr->name); // name.c -> name
            char witho[MAX_NAME];
            strcpy(witho, ptr->name); // name.c -> name.o
            witho[strlen(witho) - 1] = 'o';
            without[strlen(without) - 2] = '\0';
            fprintf(fp, "%s:", without);
            //fprintf(fp, " %s", witho);
            for (int i = 0; i < ptr->dep_count; i++)
            {

                char cname[MAX_NAME];
                strcpy(cname, ptr->dep[i]->name);
                //.h->.o
                cname[strlen(cname) - 1] = 'o';
                fprintf(fp, " %s", cname);
            }

            fprintf(fp, "\n\tgcc");
            for (int i = 0; i < ptr->dep_count; i++)
            {
                char tname[MAX_NAME];
                strcpy(tname, ptr->dep[i]->name);
                //.h->.o
                tname[strlen(tname) - 1] = 'o';
                fprintf(fp, " %s", tname);
            }
            fprintf(fp, "\n");
        }
        pclose(fp1);

        ptr = ptr->next;
    }
    fprintf(fp, "\nclean:\n\trm -f *.o a.out");
    fclose(fp);
}
void print_structure()
{
    printf("\n===== STRUCTURE =====\n");

    struct cnode *cptr = headc;

    while (cptr != NULL)
    {
        printf("\nC FILE: %s\n", cptr->name);

        for (int i = 0; i < cptr->dep_count; i++)
        {
            struct hnode *hptr = cptr->dep[i];
            printf("  -> H FILE: %s\n", hptr->name);
            printf("     Functions:\n");

            for (int j = 0; j < hptr->dep_count; j++)
            {
                struct fnode *f = hptr->dep[j];

                if (f != NULL)
                {
                    printf("       - %s\n", f->name);

                    //  printing dependencies
                    struct fnode *dep = f->next;

                    if (dep != NULL)
                    {
                        printf("         calls:\n");

                        while (dep != NULL)
                        {
                            printf("           -> %s\n", dep->just_name);
                            dep = dep->next;
                        }
                    }

                    // self recursion
                    if (f->self)
                    {
                        printf("         (self recursion)\n");
                    }
                }
            }
        }

        cptr = cptr->next;
    }

    printf("\n=====================\n");
}

void write_unused_headers(FILE *fp, struct hnode **visited, int vcount)
{
    //checking if there are any unvisited headers
    struct hnode *hptrr=headh;
    while(hptrr!=NULL)
    {
        int flag=0;
        for(int y=0;y<vcount;y++)
        {
            if(hptrr==visited[y])
            {
                flag=1;
                break;
            }
        }
        if(flag==0)
        {
            //printf("\n%s is not used anywhere\n",hptrr->name);
            // mark unused header in red
            //removed reopening dep1.dot repeatedly; write to the existing graph file.
            fprintf(fp, "\t\"%s\" [style=filled, color=red, fillcolor=lightyellow, fontcolor=red];\n", hptrr->name);// filled color should be yellow for .h
        }
        hptrr=hptrr->next;
    }
}

void write_undefined_functions(FILE *fp)
{
    //coloring undefined functions in red
    struct hnode *hp=headh;
    while(hp!=NULL)
    {
        for(int g=0;g<hp->dep_count;g++)
        {
            struct fnode *funcy=hp->dep[g];
            if(!funcy->visited)
            {
                // mark unudefined function in red
                //removed reopen dep1.dot repeatedly; write to the existing graph file.
                fprintf(fp, "\t\"%s\" [color=red, fontcolor=red];\n", funcy->name);
            }
        }
        hp=hp->next;
    }
}

//coloring graph .c - blue, .h - yellow, .f - green
void dependency_graph(int n)
{
    FILE *fp=fopen("dep1.dot","w");
    if(fp == NULL)
    {
        perror("fopen failed");
        return;
    }
    fprintf(fp,"digraph G {\n");
    struct cnode *cptr=headc;
    struct hnode *visited[n];  

    for(int i = 0; i < n; i++)
    visited[i] = NULL;


    int vcount = 0;
    char *fvisited[MAX_FUNC];
    int fcount=0;

    while(cptr!=NULL)
    {
        write_dot_node(fp,cptr->name,"lightblue");
        for(int i=0;i<cptr->dep_count;i++)
        {
            write_dot_edge(fp,cptr->name,cptr->dep[i]->name);
            
            
            int flag=0;
            for(int t=0;t<vcount;t++)
            {
                if(visited[t]==cptr->dep[i])
                {
                    flag=1;
                    break;
                }
            }
            if(flag==0)
            {
                
                write_dot_node(fp,cptr->dep[i]->name,"lightyellow");
                
                for(int j=0;j<cptr->dep[i]->dep_count;j++)
                {
                    struct fnode *func = cptr->dep[i]->dep[j];
                    write_dot_edge(fp,cptr->dep[i]->name,cptr->dep[i]->dep[j]->name);
                    int flag2=0;
                    for(int tr=0;tr<fcount;tr++)
                    {
                        if(strcmp(fvisited[tr],cptr->dep[i]->dep[j]->just_name)==0)
                            flag2=1;
                        
                    }
                    if(func->self == 1)
                    {
                        write_dot_edge(fp,func->name,func->name);
                    }
                    if(flag2==0)
                    {
                        write_dot_node(fp,cptr->dep[i]->dep[j]->name,"lightgreen");
                        fvisited[fcount++] = cptr->dep[i]->dep[j]->just_name;
                        struct fnode *fptr=cptr->dep[i]->dep[j]->next;
                        while(fptr!=NULL)
                        {
                            write_dot_edge(fp,func->name,fptr->name);
                            int flag3 = 0;
                            for(int tr=0; tr<fcount; tr++)
                            {
                                if(strcmp(fvisited[tr], fptr->just_name) == 0)
                                    flag3 = 1;
                            }

                            if(flag3 == 0)
                            {
                                write_dot_node(fp,fptr->name,"lightgreen");
                                fvisited[fcount++] = fptr->just_name;
                            }
                            fptr=fptr->next;
                            
                        }
                    }
                    
                }
                visited[vcount++]=cptr->dep[i];
            }

        }
        cptr=cptr->next;
    }
    
    write_unused_headers(fp,visited,vcount);
    write_undefined_functions(fp);
    main_processing(fp);

    //closing the graph once after all graph lines are printed.
    fprintf(fp,"}");
    fclose(fp);

}
void free_all()
{
    struct cnode *cptr=headc;
    struct cnode *tempc=headc;
    while(cptr!=NULL)
    {
        tempc=cptr;
        cptr=cptr->next;
        free(tempc->name);
        free(tempc->dep);
        free(tempc);
    }

    struct hnode *hptr=headh;
    struct hnode *temph=headh;
    while(hptr!=NULL)
    {
        for(int j=0;j<hptr->dep_count;j++)
        {
            struct fnode *fptr=hptr->dep[j];
            struct fnode *tempf=fptr;
                    
            while(tempf!=NULL)
            {
                fptr=tempf;
                tempf=fptr->next;

                free(fptr->name);
                free(fptr->just_name);
                free(fptr);
            }
        }
        temph=hptr;

        hptr=hptr->next;

        free(temph->dep);
        free(temph->name);
        free(temph);
    }
    headc=NULL;
    headh=NULL;
}


int main()
{
    int noofhfiles = hinitializer();
    
    cinitializer(noofhfiles);
    chlinker();

    mark_definitions();
    fdeplinker();
    
    //print_structure();

    dependency_graph(noofhfiles);
    

    makefile();
    //removes file extra
    remove("extra");

    system("dot -Tpng dep1.dot -o dep1.png");

    free_all();

    return 0;
}