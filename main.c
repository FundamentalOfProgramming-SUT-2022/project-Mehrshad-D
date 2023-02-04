#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#define max_input 10000


struct file{
    char path[max_input];
    char contents[max_input];
};
struct file files[10];
int file_counter = 0;
char clipboard[max_input] = {'\0'};
int pos1_global;
int pos2_global;
int line_counter_forGrep = 0;


char* cat(char*,int);
char* prepare_to_write_R(char*,int,int,int,char);
void init_null(char*);
char* prepare_to_write_I(char*,char*,int,int);
void write_stuff(char*,char*);
int find(char*,char*,int,int,int,int);
int findpos(char*,int);
void copy_for_undo(char*);

int grep(char *address,char *str,int c,int l)
{

    int index = -1;
    for(int i=1;i<=find(address,str,1,0,0,0);i++)
    {

        index = find(address,str,0,i,0,0);
        if(index == -1)return -1;
        else
        {
            line_counter_forGrep++;
            if(c == 0 && l == 0){
                findpos(address,index);
                char ad[max_input] = ".";
                strcat(ad,address);
                printf("%s:",address);
                print_line(ad,pos1_global);
                printf("\n");
            }
            if(l == 1)
            {
                printf("%s\n",address);
            }
        }
    }
    return index;
}

char* delcharfromStr(char *str1,int pos)
{
    int len = strlen(str1);
    for(int i=pos;i<len;i++)
    {
        str1[i] = str1[i+1];
    }
    return str1;
}


char* addChartoString(char *str1,char ch,int pos)
{
    int len = strlen(str1);

    for(int i=len;i>=pos+1;i--)
    {
        str1[i+1] = str1[i];
    }
    str1[pos+1] = ch;

	return str1;
}

char* auto_indent(char *address)
{
    char *infile = cat(address,0);
    int tab_counter = 1;

    int i=0;
    while(!(infile[i] == EOF || infile[i] == '\0'))
    {
        if(infile[i] == '{')
        {
            if(infile[i-1] == ' ')
            {   int m = i-2;
                while(infile[m] == ' '){delcharfromStr(infile,m);m--;i--;}
            }

            if(infile[i+1] == ' ')
            {   int m = i+1;
                while(infile[m] == ' '){delcharfromStr(infile,m);m++;}
            }
            addChartoString(infile,'\n',i);
            i++;
            for(int j=0;j<tab_counter;j++)
            {
                addChartoString(infile,'\t',i);
                i++;
            }
            tab_counter++;
        }
        if(infile[i] == '}')
        {
            if(infile[i-1] == ' ')
            {   int m = i-1;
                while(infile[m] == ' '){delcharfromStr(infile,m);m--;i--;}
            }

            if(infile[i+1] == ' ')
            {
                int m = i+1;
                while(infile[m] == ' '){delcharfromStr(infile,m);m++;}
            }
            i--;
            addChartoString(infile,'\n',i);
            i++;
            for(int j=0;j<tab_counter-2;j++)
            {
                addChartoString(infile,'\t',i);
                i++;
            }
            tab_counter--;
            i++;
            addChartoString(infile,'\n',i);
        }
        i++;
    }
    return infile;
}


void tree(char *address, const int root,int depth)
{
    int i;
    char path[1000];
    struct dirent *dp;
    DIR *dir = opendir(address);

    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL && root != 2*depth)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            for (i=0; i<root; i++)
            {
                if (i%2 == 0 || i == 0)
                    printf("%c", 179);
                else
                    printf(" ");

            }

            printf("%c%c%s\n", 195, 196, dp->d_name);

            strcpy(path, address);
            strcat(path, "/");
            strcat(path, dp->d_name);
            tree(path, root + 2,depth);
        }
    }

    closedir(dir);
}

int findpos(char *address,int index)
{
    if(index == -1){printf("string not found!\n");return -1;}
    char *infile = cat(address,0);
    int pos1 = 1;
    int pos2 = -1;
    int counter = 0;
    int i=0;
    while(counter != index)
    {
        if(*(infile+i) == '\n')
        {
            pos1++;
            pos2 = -1;
        }
        if(*(infile+i) != '\n'){counter++;pos2++;}
        i++;
    }
    pos1_global = pos1;
    pos2_global = pos2;
    return 1;

}


void replace(char *address,char *str1,char *toReplace,int at)
{
    //remove str1 from the file
    int size = strlen(str1);
    int flag = findpos(address,find(address,str1,0,at,0,0));
    if(flag == -1){printf("String doesn't exist!\n");return;}
    char *new_text;
    new_text = prepare_to_write_R(address,pos1_global,pos2_global,size,'f');
    write_stuff(address,new_text);
    //insert str2 to the file
    char *to_insert;
    to_insert = prepare_to_write_I(address,toReplace,pos1_global,pos2_global);
    if(to_insert != NULL)write_stuff(address,to_insert);
    //printf("replace done!\n");
    init_null(clipboard);


}


int line_counter(char *address)
{
    int lines = 1;
    char a;
    FILE *file = fopen(address,"r");
    while((a = fgetc(file)) != EOF)
    {
        if(a == '\n')lines++;
    }
    fclose(file);
    return lines;

}

void print_line(char *address,int line)
{
    int counter = 1;
    FILE *file = fopen(address,"r");
    char a;
    while((a = fgetc(file)) != EOF)
    {
        if(counter == line)
        {
            printf("%c",a);
        }
        if(a == '\n')counter++;
        if(counter > line)break;
    }

}

void compare(char *file1,char *file2)
{
    char address1[max_input] = ".";
    char address2[max_input] = ".";
    strcat(address1,file1);
    strcat(address2,file2);
    int f1_lines = line_counter(address1);
    int f2_lines = line_counter(address2);
    int min = (f1_lines < f2_lines) ? f1_lines : f2_lines;


    FILE *f1 = fopen(address1,"r");if(f1 == NULL){printf("file 1 doesn't exist!\n");return;}
    FILE *f2 = fopen(address2,"r");if(f2 == NULL){printf("file 2 doesn't exist!\n");return;}
    char a;
    char b;

    for(int i=1;i<=min;i++)
    {
        a = fgetc(f1);
        b = fgetc(f2);

        while(a != '\n' && b != '\n')
        {
            if(a != b)
            {
                printf("===========#%d#==========\n",i);
                print_line(address1,i);
                if(i == min)printf("\n");
                print_line(address2,i);
                if(i == min)printf("\n\n");
                break;
            }
            a = fgetc(f1);
            b = fgetc(f2);
        }

        if((a != '\n' && b == '\n') || (b != '\n' && a == '\n'))
        {
            printf("===========#%d#==========\n",i);
            print_line(address1,i);
            print_line(address2,i);
        }
        while(a != '\n' && a != EOF)a = fgetc(f1);
        while(b != '\n' && b != EOF)b = fgetc(f2);
    }
    if(f1_lines<f2_lines)
    {
        printf("second file is longer. here is the rest\n");
        for(int i=min+1;i<=f2_lines;i++)print_line(address2,i);
        printf("\n");

    }
    else if(f1_lines>f2_lines)
    {
        printf("first file is longer. here is the rest\n");
        for(int i=min+1;i<=f1_lines;i++)print_line(address1,i);
        printf("\n");

    }
    else
    {
        printf("files have equal lines\n");
    }
    fclose(f1);
    fclose(f2);

}


void init_null(char *a)
{   int i=0;
    while (*(a+i) != '\0'){*(a+i) = '\0';i++;}
}

void copy_for_undo(char *address)
{

    char a;
    char undo[max_input] = {'\0'};
    char ad[max_input] = ".";
    for(int i=0;i<file_counter;i++)
    {
        if(!strcmp(address,files[i].path))
        {
            //copy for undo
            strcat(ad,address);
            FILE *file = fopen(ad,"r");
            while((a = fgetc(file)) != EOF){strncat(undo,&a,1);}
            fclose(file);//

            strcpy(files[i].contents,undo);
            return;
        }
    }
    strcpy(files[file_counter].path,address);
    strcat(ad,address);
    //copy for undo
    FILE *file = fopen(ad,"r");
    while((a = fgetc(file)) != EOF){strncat(undo,&a,1);}
    fclose(file);//

    strcpy(files[file_counter].contents,undo);
    file_counter++;
}


char* prepare_to_write_R(char *address,int line,int column,int size,char b_or_f)//remove and also copy to clipboard
{
    char *enter = "\n";
    char *contents = (char*)malloc(max_input * sizeof(char));
    char ad[max_input] = ".";
    char a;
    strcat(ad,address);
    /*//copy for undo
    FILE *file = fopen(ad,"r");
    while((a = fgetc(file)) != EOF){strncat(undo,&a,1);}
    fclose(file);//*/

    FILE *file = fopen(ad,"r");
    if(file == NULL){printf("file doesn't exist!\n");return NULL;}

    //before the pos
    for(int i=0;i<line-1;i++)
    {
        while((a = fgetc(file)) != '\n')
        {
            if(a == EOF){printf("position doesn't exist!!\n");return NULL;}
            strncat(contents,&a,1);
        }

        strncat(contents,enter,1);

    }
    if(b_or_f == 'b')
    {
        for(int i=0;i<column-size;i++)
        {
            a = fgetc(file);
            if(a == EOF){printf("position doesn't exist!!\n");return NULL;}
            strncat(contents,&a,1);
        }
        for(int i=0;i<size;i++)
        {
            a = fgetc(file);
            if(a == EOF){printf("position doesn't exist!!\n");return NULL;}
            clipboard[i] = a;

        }
        while((a = fgetc(file)) != EOF)
        strncat(contents,&a,1);
        fclose(file);
        return contents + 1;


    }
    else
    {
        for(int i=0;i<column;i++)
        {
            a = fgetc(file);
            if(a == EOF){printf("position doesn't exist!!\n");return NULL;}
            strncat(contents,&a,1);
        }
        for(int i=0;i<size;i++)
        {
            a = fgetc(file);
            if(a == EOF){printf("position doesn't exist!!\n");return NULL;}
            clipboard[i] = a;
        }
        while((a = fgetc(file)) != EOF)
            strncat(contents,&a,1);
        fclose(file);
        return contents +1;


    }

}


char* prepare_to_write_I(char *address,char *stuff,int line,int column)
{
    char *enter = "\n";
    char *contents = (char*)malloc(max_input * sizeof(char));
    char ad[max_input] = ".";
    char a;
    strcat(ad,address);

    /*//copy for undo
    FILE *file = fopen(ad,"r");
    while((a = fgetc(file)) != EOF){strncat(undo,&a,1);}
    fclose(file);//*/

    FILE *file = fopen(ad,"r");
    if(file == NULL){printf("file doesn't exist!\n");return NULL;}

    //before the pos
    for(int i=0;i<line-1;i++)
    {
        while((a = fgetc(file)) != '\n')
        {
            if(a == EOF){printf("position doesn't exist\n!!");return NULL;}
            strncat(contents,&a,1);
        }

        strncat(contents,enter,1);

    }
    for(int i=0;i<column;i++)
    {
        a = fgetc(file);
        if(a == EOF){printf("position doesn't exist!!\n");return NULL;}
        strncat(contents,&a,1);
    }
    //the pos
    strcat(contents,stuff);
    //after the pos
    while((a = fgetc(file)) != EOF)
        strncat(contents,&a,1);
    fclose(file);

    return contents+1;
}

char* cat(char *address,int print_ornot)
{
    char ad[max_input] = ".";
    strcat(ad,address);
    char *stuff_in_file = (char*)calloc(100000,sizeof(char));//[100000] = {'\0'};
    FILE *file = fopen(ad,"r");
    if(file != NULL)
    {
        char a;
        while((a = fgetc(file)) != EOF)
            strncat(stuff_in_file,&a,1);
        fclose(file);

        if(print_ornot == 1)printf("%s\n",stuff_in_file);
        return stuff_in_file;

    }
    else
    {
        printf("file doesn't exist!\n");
    }


}


int find(char *address,char *string,int count,int at,int byword,int flag)
{
    char *in_file = cat(address,0);
    int len1 = strlen(string);
    int len2 = strlen(in_file);
    int matched = 0;
    int find_counter = 0;
    int word_counter = 0;
    int enter_counter = 0;
    int index = -1;
    int realindex = -1;

    for(int i=0;i<=len2-len1;i++)
    {
        if((in_file[i] == ' ' || in_file[i] == '\n') && find_counter == 0) word_counter++;
        if(in_file[i] == '\n')enter_counter++;

        for(int j=i,k=0;j<=i+len1-1;j++,k++)
        {
            if(in_file[j] == string[k])
            {
                matched++;
            }
            else
            {
                break;
            }
        }
        if(matched == len1)
        {
            find_counter++;
            if(find_counter == at)
            {
                index = i - enter_counter + 1;
                realindex = i;
                //break;
            }
            matched = 0;
        }
        else
        {
            matched = 0;
        }
    }


    if(flag == 1)
    {   if(index == -1)return -1;
        int t = realindex;
        while(!(in_file[t] == ' ' || in_file[t] == '\n' || t == 0))t--;
        return t+1+index-realindex;
    }
    if(at != 0)return index;
    if(count != 0)return find_counter;
    if(byword != 0)return word_counter+1;


}

void write_stuff(char *address,char *stuff)
{
    char ad[max_input] = ".";
    strcat(ad,address);
    FILE *file = fopen(ad,"w");
    if(file == NULL){printf("file doens't exist!\n");return;}
    fprintf(file,"%s",stuff);
    fclose(file);
    //printf("successful!!\n");
}

void del_quotation(char *a)
{
    int i =1;
    while(*(a+i) != '\0')
    {
        if(*(a+i) == '"')
        {
            *(a+i) = '\0';
        }
        i++;
    }
}


int slash_counter(char *a)
{
    int counter = 0;
    int i=7;
    while(*(a+i) != '\0')
    {
        if(*(a+i) == '/')
            counter++;
        i++;
    }
    return counter;
}

char* split(char *a,int n)
{
    char *input_copy = (char*)malloc(max_input * sizeof(char));
    strcpy(input_copy,a);
    int m = 0;
    int i = 6;
    while(*(input_copy+i) != '\0')
    {
        if(*(input_copy+i) == '/')
        {
            m++;
           if(n == m)
           {
               *(input_copy+i) = '\0';
               return input_copy;
           }
        }
        i++;
    }
    return a;
}

int isDirectoryExists(const char *path)
{
    struct stat stats;

    stat(path, &stats);

    // Check for file existence
    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}

void createfolder(char *address)
{
    if(isDirectoryExists(address))
    {
        printf("folder exists!!\n");
    }
    else
    {
        mkdir(address);
        //printf("folder is succesfully made!\n");
    }

}

void createfile(char *address)
{
    FILE *my_file = fopen(address,"r");
    if(my_file != NULL)
    {
        printf("file exists!!\n");
        fclose(my_file);
    }
    else
    {
        my_file = fopen(address,"w");
        fclose(my_file);
        printf("file is succesfully made!\n");
    }

}

int main()
{
    char main_clipboard[max_input] = {'\0'};

    char *input = (char*)malloc(max_input * sizeof(char));

    while(1){
    scanf("%s",input);

    if(!strcmp(input,"createfile"))
    {
        char buff[10];
        char fileaddress[max_input];
        scanf("%s ",buff);
        gets(fileaddress);
        if(*fileaddress == '/')
        {
            int slashes = slash_counter(fileaddress);
            for(int i=1;i<=slashes;i++)
            {
                char current[max_input] = ".";
                strcat(current,split(fileaddress,i));
                createfolder(current);
            }
            char current[max_input] = ".";
            createfile(strcat(current,fileaddress));

        }
        else
        {
            int slashes = slash_counter(fileaddress);
            del_quotation(fileaddress);
            for(int i=1;i<=slashes;i++)
            {
                char current[max_input] = ".";
                strcat(current,split(fileaddress+1,i));
                createfolder(current);
            }
            char current[max_input] = ".";
            createfile(strcat(current,fileaddress+1));

        }


    }
    else if(!strcmp(input,"insertstr"))
    {
        char fileaddress[max_input] = {'\0'};
        char str_to_insert[max_input] = {'\0'};
        int pos1,pos2;
        char buff[100];


        scanf("%s ",buff);
        char a = getchar();
        if(a == '/')
        {
            *fileaddress = '/';
            scanf("%s",fileaddress+1);
            scanf("%s ",buff);
            char b = getchar();
            if(b != '"')
            {
                *str_to_insert = b;
                scanf("%s",str_to_insert+1);
                scanf("%s ",buff);
                scanf("%d:%d",&pos1,&pos2);
            }
            else
            {   int i=0;
                while(1)
                {
                    str_to_insert[i] = getchar();
                    if(str_to_insert[i] == 'n' && str_to_insert[i-1] == '\\' && str_to_insert[i-2] != '\\')
                    {
                        str_to_insert[i-1] = '\n';
                        i--;

                    }
                    if(str_to_insert[i] == 'n' && str_to_insert[i-1] == '\\' && str_to_insert[i-2] == '\\')
                    {
                        str_to_insert[i-1] = 'n';
                        i--;

                    }
                    if(str_to_insert[i] == ' ' && str_to_insert[i-1] == '"')
                    {
                        str_to_insert[i-1] = '\0';
                        break;
                    }
                    i++;
                }
                scanf("%s ",buff);
                scanf("%d:%d",&pos1,&pos2);
            }



        }
        else
        {
            int i=0;
            while(1)
            {
                fileaddress[i] = getchar();
                if(fileaddress[i] == ' ' && fileaddress[i-1] == '"')
                {
                    fileaddress[i-1] = '\0';
                    break;
                }
                i++;
            }
            scanf("%s ",buff);
            char b = getchar();
            if(b != '"')
            {
                *str_to_insert = b;
                scanf("%s",str_to_insert+1);
                scanf("%s ",buff);
                scanf("%d:%d",&pos1,&pos2);
            }
            else
            {   int i=0;
                while(1)
                {
                    str_to_insert[i] = getchar();
                    if(str_to_insert[i] == 'n' && str_to_insert[i-1] == '\\' && str_to_insert[i-2] != '\\')
                    {
                        str_to_insert[i-1] = '\n';
                        i--;

                    }
                    if(str_to_insert[i] == 'n' && str_to_insert[i-1] == '\\' && str_to_insert[i-2] == '\\')
                    {
                        str_to_insert[i-1] = 'n';
                        i--;

                    }
                    if(str_to_insert[i] == ' ' && str_to_insert[i-1] == '"')
                    {
                        str_to_insert[i-1] = '\0';
                        break;
                    }
                    i++;
                }
                scanf("%s ",buff);
                scanf("%d:%d",&pos1,&pos2);
            }

        }//end input
        copy_for_undo(fileaddress);
        char *to_insert;/// = (char*)malloc(100000 * sizeof(char));
        to_insert = prepare_to_write_I(fileaddress,str_to_insert,pos1,pos2);
        if(to_insert != NULL)write_stuff(fileaddress,to_insert);
        printf("Insert Successful!\n");


    }
    else if(!strcmp(input,"cat"))
    {
        char fileaddress[max_input] = {'\0'};
        char buff[100];

        scanf("%s ",buff);

        gets(fileaddress);
        if(*fileaddress == '/')
        {
            cat(fileaddress,1);
        }
        else
        {
            del_quotation(fileaddress);
            cat(fileaddress+1,1);
        }

    }
    else if(!strcmp(input,"removestr"))
    {
        //take input
        char fileaddress[max_input] = {'\0'};
        char buff[100];
        char b_f;
        int pos1,pos2,size;

        scanf("%s ",buff);
        char a = getchar();

        if(a == '/')
        {
            *fileaddress = '/';
            scanf("%s",fileaddress+1);
            scanf("%s ",buff);
            scanf("%d:%d",&pos1,&pos2);
            getchar();
            scanf("%s ",buff);
            scanf("%d",&size);getchar();getchar();
            b_f = getchar();
        }
        else
        {
            int i=0;
            while(1)
            {
                fileaddress[i] = getchar();
                if(fileaddress[i] == ' ' && fileaddress[i-1] == '"')
                {
                    fileaddress[i-1] = '\0';
                    break;
                }
                i++;
            }
            scanf("%s ",buff);
            scanf("%d:%d",&pos1,&pos2);
            getchar();
            scanf("%s ",buff);
            scanf("%d",&size);getchar();getchar();
            b_f = getchar();
        }//input finished
        copy_for_undo(fileaddress);
        char *new_text;
        new_text = prepare_to_write_R(fileaddress,pos1,pos2,size,b_f);
        init_null(clipboard);
        write_stuff(fileaddress,new_text);
        printf("Remove Successful!\n");

    }
    else if(!strcmp(input,"copystr"))
    {
        //take input
        char fileaddress[max_input] = {'\0'};
        char buff[100];
        char b_f;
        int pos1,pos2,size;

        scanf("%s ",buff);
        char a = getchar();

        if(a == '/')
        {
            *fileaddress = '/';
            scanf("%s",fileaddress+1);
            scanf("%s ",buff);
            scanf("%d:%d",&pos1,&pos2);
            getchar();
            scanf("%s ",buff);
            scanf("%d",&size);getchar();getchar();
            b_f = getchar();
        }
        else
        {
            int i=0;
            while(1)
            {
                fileaddress[i] = getchar();
                if(fileaddress[i] == ' ' && fileaddress[i-1] == '"')
                {
                    fileaddress[i-1] = '\0';
                    break;
                }
                i++;
            }
            scanf("%s ",buff);
            scanf("%d:%d",&pos1,&pos2);
            getchar();
            scanf("%s ",buff);
            scanf("%d",&size);getchar();getchar();
            b_f = getchar();
        }//input finished

        prepare_to_write_R(fileaddress,pos1,pos2,size,b_f);//copy to clipboard
        copy_for_undo(fileaddress);
        strcpy(main_clipboard,clipboard);//make sure the clip board doesn't change!
        init_null(clipboard);//initialize clip board to null
        printf("ready to paste!!\n");


    }
    else if(!strcmp(input,"cutstr"))
    {
        //take input
        char fileaddress[max_input] = {'\0'};
        char buff[100];
        char b_f;
        int pos1,pos2,size;

        scanf("%s ",buff);
        char a = getchar();

        if(a == '/')
        {
            *fileaddress = '/';
            scanf("%s",fileaddress+1);
            scanf("%s ",buff);
            scanf("%d:%d",&pos1,&pos2);
            getchar();
            scanf("%s ",buff);
            scanf("%d",&size);getchar();getchar();
            b_f = getchar();
        }
        else
        {
            int i=0;
            while(1)
            {
                fileaddress[i] = getchar();
                if(fileaddress[i] == ' ' && fileaddress[i-1] == '"')
                {
                    fileaddress[i-1] = '\0';
                    break;
                }
                i++;
            }
            scanf("%s ",buff);
            scanf("%d:%d",&pos1,&pos2);
            getchar();
            scanf("%s ",buff);
            scanf("%d",&size);getchar();getchar();
            b_f = getchar();
        }//input finished
        copy_for_undo(fileaddress);
        char *new_content = prepare_to_write_R(fileaddress,pos1,pos2,size,b_f);//copied to clipboard in this function
        strcpy(main_clipboard,clipboard);//make sure the clip board doesn't change!
        init_null(clipboard);
        write_stuff(fileaddress,new_content);
        printf("ready to paste!!\n");

    }
    else if(!strcmp(input,"pastestr"))
    {
        //take input
        char fileaddress[max_input] = {'\0'};
        char buff[100];
        int pos1,pos2;

        scanf("%s ",buff);
        char a = getchar();

        if(a == '/')
        {
            *fileaddress = '/';
            scanf("%s",fileaddress+1);
            scanf("%s ",buff);
            scanf("%d:%d",&pos1,&pos2);
        }
        else
        {
            int i=0;
            while(1)
            {
                fileaddress[i] = getchar();
                if(fileaddress[i] == ' ' && fileaddress[i-1] == '"')
                {
                    fileaddress[i-1] = '\0';
                    break;
                }
                i++;
            }
            scanf("%s ",buff);
            scanf("%d:%d",&pos1,&pos2);
        }//input finished
        copy_for_undo(fileaddress);
        char *new_content = prepare_to_write_I(fileaddress,main_clipboard,pos1,pos2);
        write_stuff(fileaddress,new_content);
        printf("pasted successfully!!\n");
    }
    else if(!strcmp(input,"find"))
    {
        char *fileaddress = (char*)malloc(max_input * sizeof(char));
        char *string = (char*)malloc(max_input * sizeof(char));
        char buff[10];
        int count = 0,at = 0,byword = 0,all = 0;
        //take input
        scanf("%s ",buff);
        char a = getchar();
        if(a == '"')
        {
            int i=0;
            while(1)
            {
                string[i] = getchar();
                if(string[i] == ' ' && string[i-1] == '"')
                {
                    string[i-1] = '\0';
                    break;
                }
                i++;
            }
        }
        else
        {
            *string = a;
            scanf("%s",string+1);
        }
        scanf("%s ",buff);
        a = getchar();
        if(a == '"')
        {
            int i=0;
            while(1)
            {
                fileaddress[i] = getchar();
                if((fileaddress[i] == ' ' || fileaddress[i] == '\n') && fileaddress[i-1] == '"')
                {
                    fileaddress[i-1] = '\0';
                    break;
                }
                i++;
            }
            if(fileaddress[i] == ' '){
                char a;char b;
                getchar();
                a= getchar();b = getchar();
                if(a == 'c'){count = 1;scanf("%s",buff);}
                else if(a == 'b'){byword = 1;scanf("%s",buff);}
                else if(a == 'a' && b != 'l'){scanf("%d",&at);}
                else {all = 1;scanf("%s",buff);}
            }

        }
        else
        {
            *fileaddress = '/';
            int i=1;
            while(1)
            {
                fileaddress[i] = getchar();
                if(fileaddress[i] == ' ' || fileaddress[i] == '\n')
                {
                    if(fileaddress[i] == ' '){
                char a;char b;
                getchar();
                a= getchar();b = getchar();
                if(a == 'c'){count = 1;scanf("%s",buff);}
                else if(a == 'b'){byword = 1;scanf("%s",buff);}
                else if(a == 'a' && b != 'l'){scanf("%d",&at);}
                else {all = 1;scanf("%s",buff);}

            }
                    fileaddress[i] = '\0';
                    break;
                }
                i++;
            }
        }//input finished

        int length = strlen(string);

        if(string[0] != '*')
        {
            if(string[length-1] == '*')string[length-1] = '\0';
            if(all == 0)
            {
                if(at == 0 && byword == 0 && count == 0)at = 1;
                printf("%d\n",find(fileaddress,string,count,at,byword,0));
            }
            else
            {
                int counter = find(fileaddress,string,1,0,0,0);
                for(int i=1;i<=counter;i++)
                {
                    printf("%d-",find(fileaddress,string,0,i,0,0));
                }
                printf("\n");
            }
        }
        else
        {

            if(all == 0)
            {
                if(at == 0 && byword == 0 && count == 0)
                {
                    at = 1;
                    printf("%d\n",find(fileaddress,string+1,count,at,byword,1));
                }
                else if(count !=0 || byword !=0)
                    printf("%d\n",find(fileaddress,string+1,count,at,byword,0));
                else
                    printf("%d\n",find(fileaddress,string+1,count,at,byword,1));

            }
            else
            {
                int counter = find(fileaddress,string+1,1,0,0,0);
                for(int i=1;i<=counter;i++)
                {
                    printf("%d-",find(fileaddress,string+1,0,i,0,1));
                }
                printf("\n");
            }

        }

    }
    else if(!strcmp(input,"replace"))
    {
        char fileaddress[max_input];
        char str1[max_input];
        char str2[max_input];
        char buff[20];
        int at = 1;
        int all = 0;

        scanf("%s ",buff);
        char a = getchar();
        if(a == '"')
        {
            int i=0;
            while(1)
            {
                str1[i] = getchar();
                if(str1[i] == ' ' && str1[i-1] == '"')
                {
                    str1[i-1] = '\0';
                    break;
                }
                i++;
            }
        }
        else
        {
            *str1 = a;
            scanf("%s",str1+1);
        }

        scanf("%s ",buff);
        a = getchar();
        if(a == '"')
        {
            int i=0;
            while(1)
            {
                str2[i] = getchar();
                if(str2[i] == ' ' && str2[i-1] == '"')
                {
                    str2[i-1] = '\0';
                    break;
                }
                i++;
            }
        }
        else
        {
            *str2 = a;
            scanf("%s",str2+1);
        }
        scanf("%s ",buff);

        a = getchar();
        if(a == '"')
        {
            int i=0;
            while(1)
            {
                fileaddress[i] = getchar();
                if((fileaddress[i] == ' ' || fileaddress[i] == '\n') && fileaddress[i-1] == '"')
                {
                    fileaddress[i-1] = '\0';
                    break;
                }
                i++;
            }
            if(fileaddress[i] == ' '){
                char a;char b;
                getchar();
                a= getchar();b = getchar();
                if(a == 'a' && b != 'l'){scanf("%d",&at);}
                else {all = 1;scanf("%s",buff);}
            }

        }
        else
        {
            *fileaddress = '/';
            int i=1;
            while(1)
            {
                fileaddress[i] = getchar();
                if(fileaddress[i] == ' ' || fileaddress[i] == '\n')
                {
                    if(fileaddress[i] == ' '){
                        char a;char b;
                        getchar();
                        a= getchar();b = getchar();
                        if(a == 'a' && b != 'l'){scanf("%d",&at);}
                        else {all = 1;scanf("%s",buff);}
                        }
                    fileaddress[i] = '\0';
                    break;
                }
                i++;
            }
        }//input finished

        copy_for_undo(fileaddress);

        if(all == 0)
        {
            replace(fileaddress,str1,str2,at);
        }
        else
        {
            int counter = find(fileaddress,str1,1,0,0,0);
            for(int i = 1;i<=counter;i++)
            {
                replace(fileaddress,str1,str2,1);
            }
        }

    }
    else if(!strcmp(input,"grep"))
    {
        char fileaddresses[10][max_input];
        char buff[20];
        char string[max_input];
        char a;
        int c = 0,l = 0;

        scanf("%s ",buff);
        if(buff[1] == 'c'){c=1;scanf("%s ",buff);}

        if(buff[1] == 'l'){l=1;scanf("%s ",buff);}
        a = getchar();
        if(a == '"')
        {
            int i=0;
            while(1)
            {
                string[i] = getchar();
                if(string[i] == ' ' && string[i-1] == '"')
                {
                    string[i-1] = '\0';
                    break;
                }
                i++;
            }
        }
        else
        {
            *string = a;
            int i=1;
            while(1)
            {
                string[i] = getchar();
                if(string[i] == ' ')
                {
                    string[i] = '\0';
                    break;
                }
                i++;
            }
        }
        int i=0;
        while (1)
        {
            int j=0;
            a = getchar();

            while(a != ' ' && a != '\n')
            {
                fileaddresses[i][j] = a;
                a = getchar();
                j++;
            }
            fileaddresses[i][j] = '\0';
            if(a == '\n')break;
            i++;
        }
        fileaddresses[i+1][0] = '\0';

        for(int i=0;fileaddresses[i][0] != '\0';i++)
        {
            grep(fileaddresses[i],string,c,l);
        }
        if(c == 1)
        {
            printf("%d\n",line_counter_forGrep);
        }
        line_counter_forGrep = 0;

    }
    else if(!strcmp(input,"undo"))
    {
        char fileaddress[max_input];
        char buff[100];
        char current[max_input] = ".";

        scanf("%s ",buff);
        gets(fileaddress);
        if(*fileaddress == '"')
        {
            del_quotation(fileaddress);
            for(int i=0;i<10;i++)
            {
                if(!strcmp(files[i].path,fileaddress+1))
                {
                    char *temp = cat(fileaddress+1,0);
                    write_stuff(fileaddress+1,files[i].contents);
                    strcpy(files[i].contents,temp);

                }

            }

        }
        else
        {
            for(int i=0;i<10;i++)
            {
                if(!strcmp(files[i].path,fileaddress))
                {
                    char *temp = cat(fileaddress,0);
                    write_stuff(fileaddress,files[i].contents);
                    strcpy(files[i].contents,temp);
                }

            }
        }

    }
    else if(!strcmp(input,"auto-indent"))
    {
        char fileaddress[max_input];
        scanf("%s",fileaddress);
        copy_for_undo(fileaddress);///I dont know if undo is needed for auto indent!
        char *newstuff = auto_indent(fileaddress);
        write_stuff(fileaddress,newstuff);

    }
    else if(!strcmp(input,"tree"))
    {
        int depth;
        scanf("%d",&depth);
        tree("./root",0,depth);

    }
    else if(!strcmp(input,"compare"))
    {
        char file1[max_input];
        char file2[max_input];
        scanf("%s",file1);
        scanf("%s",file2);
        compare(file1,file2);

    }
    else if(!strcmp(input,"exit"))
    {
        printf("\n");
        printf("\n");
        printf("################     <Thanks For Using>         #################\n");
        printf("\n");
        return 0;
    }
    else
    {
        printf("Invalid Input\n");
    }
    }

    return 0;
}
///DONE!
