#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int MAX_NAME = 100;
int MAX_CMD = 100;
int MAX_LINE=100;
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
                        int l=0;// to make sure we're reading content after {- opening brace
                        do// constraint ending } should be on seperate line - removed this constraint
                        {
                            //printing function code into extra
                            fprintf(fp2,"%s",name);
                            for(int y=0;y<strlen(name);y++)
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

                    //  print dependencies
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
void dependency_graph(int n)
{
    FILE *fp=fopen("dep1.dot","w");
    fprintf(fp,"digraph G {\n");
    struct cnode *cptr=headc;
    struct hnode *visited[n];  

    for(int i = 0; i < n; i++)
    visited[i] = NULL;


    int vcount = 0;

    while(cptr!=NULL)
    {
        for(int i=0;i<cptr->dep_count;i++)
        {
            fprintf(fp,"\t \"%s\"->\"%s\";\n",cptr->name,cptr->dep[i]->name);
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
                
    
                
                for(int j=0;j<cptr->dep[i]->dep_count;j++)
                {
                    struct fnode *func = cptr->dep[i]->dep[j];
                    fprintf(fp,"\t \"%s\"->\"%s\";\n",cptr->dep[i]->name,cptr->dep[i]->dep[j]->name);
                    if(func->self == 1)
                    {
                        fprintf(fp,"\t \"%s\"->\"%s\";\n",
                                func->name,
                                func->name);
                    }
                    struct fnode *fptr=cptr->dep[i]->dep[j]->next;
                    while(fptr!=NULL)
                    {
                        fprintf(fp,"\t \"%s\"->\"%s\";\n",cptr->dep[i]->dep[j]->name,fptr->name);
                        fptr=fptr->next;
                        
                    }
                }
                visited[vcount++]=cptr->dep[i];
            }

        }
        cptr=cptr->next;
    }
    
    fclose(fp);

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
            FILE *fpp=fopen("dep1.dot","a");
            // mark unused header in red
            fprintf(fpp, "\t\"%s\" [color=red, fontcolor=red];\n", hptrr->name);
            fclose(fpp);
        }
        hptrr=hptrr->next;
    }
    //coloring undefined functions in red
    struct hnode *hp=headh;
    while(hp!=NULL)
    {
        for(int g=0;g<hp->dep_count;g++)
        {
            struct fnode *funcy=hp->dep[g];
            if(!funcy->visited)
            {
                FILE *fpppp=fopen("dep1.dot","a");
                // mark unudefined function in red
                fprintf(fpppp, "\t\"%s\" [color=red, fontcolor=red];\n", funcy->name);
                fclose(fpppp);
            }
        }
        hp=hp->next;
    }
    FILE *fppp=fopen("dep1.dot","a");
    fprintf(fppp,"}");
    fclose(fppp);

}
int main()
{
    int noofhfiles = hinitializer();
    
    cinitializer(noofhfiles);
    chlinker();

    mark_definitions();
    fdeplinker();
    print_structure();

    dependency_graph(noofhfiles);
    system("dot -Tpng dep1.dot -o dep1.png");

    makefile();
    system("rm extra");

}