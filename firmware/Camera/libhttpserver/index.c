#include "common.h"

#define MAXWORDLEN 1000
char blankchars[MAXCHARDEFINED] = " \n&#:;_\\|/-+=?!@$%^'\"`~,.<>[]{}0123456789";
char englishchars[100] ="abcdefghijklmnopqrstuvwxyz";

INDEX_CONTEXT index_context = {0};
#define __INDEX_CONTEXT (&(index_context))

LOCATIONINFO * head_locate;

#define read_BUF  12
#define MAXSENTENCELEN  1024
static int count_file = 2;
/////////////////////////////////////////////////////

#define my_malloc(size) malloc(size)
#define my_realloc(p, size) realloc(p, size)
#define my_free(p) free(p)
#define my_safe_free(a) { if (a) {my_free(a); a = NULL;} }
#define my_safe_strlen(x) ((x == NULL)?0:strlen(x))

static inline unsigned char * mem_new(int value, unsigned int len)
{
	unsigned char * p = (unsigned char *)my_malloc(len);
	if (! p)
		return NULL;
	memset(p, value, len);
	return p;
}

static inline unsigned char * mem_assign(const unsigned char * value, unsigned int len)
{
	unsigned char * p = (unsigned char *)my_malloc(len);
	if (! p)
		return NULL;
	memcpy(p, value, len);
return p;
}

static inline unsigned char * mem_combine(const unsigned char * src1, unsigned int src1_len, const unsigned char * src2, unsigned int src2_len)
{
	unsigned char * p = (unsigned char *)my_malloc(src1_len + src2_len);
	if (! p)
		return NULL;
	memcpy(p, src1, src1_len);
	memcpy(p + src1_len, src2, src2_len);
	return p;
}

static inline unsigned char * mem_append(unsigned char * src, unsigned int src_len, const unsigned char * append, unsigned int append_len)
{
	unsigned char * p = mem_combine(src, src_len, append, append_len);
	my_free(src);
	return p;
}

static inline char * str_assign(const char * value)
{
	return (char *)mem_assign((unsigned char *)value, strlen(value) + 1);
}

static inline char * str_append(char * src, const char * append)
{
	return (char *)mem_append((unsigned char *)src, strlen(src), (const unsigned char *)append, strlen(append) + 1);
}

static inline char * str_combine(const char * src1, const char * src2)
{
	return (char *)mem_combine((const unsigned char *)src1, strlen(src1), (const unsigned char *)src2, strlen(src2) + 1);
}

/////////////////////////////////////////////////////

void *emalloc(int i)
{
	void *p;
	
	if ((p = (void *) malloc(i)) == NULL)
		printf("Ran out of memory (could not allocate enough)!");
	return p;
}

int isblankchar(char c)
{
	int i;
	
	for (i = 0; blankchars[i] != '\0'; i++)
	{
		if ((char)c == blankchars[i])
			return 1;
	}
	return 0;
}

int isenglishchar(char c)
{
    int i,d;
	d= tolower(c);
	for (i = 0; englishchars[i] != '\0'; i++)
	{
		if ((char)d == englishchars[i])
			return 1;
	}
	return 0;
}


char * my_strstr(const char * s1,const char * s2,int len)
{
	int n;
	if(*s2)
	{
		while(len--)
		{
			for(n=0;*(s1+n)==*(s2+n);n++)
			{
				if(!*(s2+n+1))
					return (char*)s1;
			}
			s1++;
		}
		return NULL;
	}
	else
	return (char*)s1;
}


void get_extension(const char *file_name,char *extension)
{
	int i=0,length;
	length=strlen(file_name);
	while(file_name[i])
	{
		if(file_name[i]=='.')
			break;
		i++;
	}
	if(i<length)
		strcpy(extension,file_name+i+1);
	else
		strcpy(extension,"\0");
}


char * Process_File_Type(char * file)
{
	char cmd[1024];
	char  ext[10];
	char new_file[1024];
	//判断文件的格式

	get_extension(file,ext);

	//转换文件的格式
	if(!strcmp(ext,"doc") || !strcmp(ext,"docx"))
	{


	}
	else if(!strcmp(ext,"pdf"))
	{

	}

	else if(!strcmp(ext,"txt") || !strcmp(ext,"lua") || !strcmp(ext,"sh"))
	{
		 printf("this is text ......................\n");
		sprintf(cmd,"cp %s /tmp/convert_file.txt",file);
		system(cmd);

	}
	else 
		return NULL;

	strcpy(new_file,"/tmp/convert_file.txt");
	return new_file;

}



struct locationinfo * get_word_locate(FILEINFO *f,char *word)
{
    LOCATIONINFO  * linfo = NULL;
    linfo =(LOCATIONINFO *)malloc(sizeof(LOCATIONINFO));
    
    linfo->id=f->id;
    linfo->frequency=1;
    linfo->next=NULL;
    
    return linfo;
}


void UpdateWordList(FILEINFO *f,char *word)
{
    static int count_wrodlist =0;
 
    if(__INDEX_CONTEXT->wordlist == NULL)
    {
        __INDEX_CONTEXT->wordlist = (WORDINFO *)malloc(sizeof(WORDINFO));
        __INDEX_CONTEXT->wordlist->word=str_assign(word);
        __INDEX_CONTEXT->wordlist->locate = get_word_locate(f,word);
        __INDEX_CONTEXT->wordlist->next = NULL;
    }
    else
    {
        WORDINFO *p,*q,*newword;
        p = __INDEX_CONTEXT->wordlist;
        q = __INDEX_CONTEXT->wordlist;
        int flag=0;

        while( p !=NULL)
        { 
            if(strncmp(p->word,word,strlen(word)) ==0) 
            {
            
                flag = 1;
                LOCATIONINFO *locate_p,*locate_q,*locate_new;
                
                locate_p = p->locate;
                locate_q = p->locate;

              // while(locate_p->id != f->id && locate_p->next != NULL)
                while(locate_p !=NULL)
                {
                    if(locate_p->id == f->id)          
                        break;

                    locate_q = locate_p;
                    locate_p = locate_p->next;
       
                }

                if(locate_p != NULL)
                {
                    locate_p->frequency++;
                }
                else
                {
                   locate_q->next = get_word_locate(f,word);

                }

                break;
            }
        
            q = p;
            p = p->next;
        }

        if(flag == 0)
        {

            newword = (WORDINFO *)malloc(sizeof(WORDINFO));

            newword->word=str_assign(word);
            newword->locate=get_word_locate(f,word);
            newword->next=NULL;

            q->next=newword;
          //  printf("wordlist=%s %d \n",word,count_wrodlist++);
        }
    }
  
}

int query_dictionary(char *str,char * word)
{
    if(my_strstr(str,word,strlen(str)))
    //if(strstr(str,word))
    {
        return 1;
    }
    else
        return 0;
}

int query_englishword(FILEINFO *f,char *buf)
{
        int i=0 ,j;
        static int englishcount =0;
        int sentence = 0;
        char *str = buf;
        int len_org = strlen(str); 
        int encount = 0;
        char enword[24];
       //  printf("str=%s \n",str);
    
        for(j=0;j<len_org+1;j++)
        {
            str[j]= tolower(str[j]);
            if(!sentence)
            {
                if(!isenglishchar(str[j]))
                    continue;
                else
                {
                    i = 0;
                    enword[i++] = str[j]; 
                    sentence = 1;
                    
                }                        
            }
            else if(sentence) 
            {
                if(!isenglishchar(str[j]))
                { 
                
                    enword[i++] = '\0';
                    sentence = 0;
                    

                    if(strlen(enword)>3)
                    {
                       // printf("enword in str=%s +%s\n",enword,str);
                        UpdateWordList(f,enword);
                        memset(enword,0,24);
                    }
                }
                else
                {
                    enword[i++] = str[j];  

                }
            }  
            
        }
}

void printfwordlist(WORDINFO *wlist)
{
    WORDINFO *p,*q;
    LOCATIONINFO *locate_p,*locate_q;
   
    p=q=__INDEX_CONTEXT->wordlist;
    
    
    while(p != NULL)
    {
        if(strcmp(p->word,"之后") == 0)
        { 
            locate_p= locate_q=p->locate;;
            while(locate_p != NULL)
            {
                printf("# id=%d $$ frequency=%d\n",locate_p->id,locate_p->frequency);
                locate_p=locate_p->next;
            }
        }
        p=p->next;
    }
    
    
}

void WriteIndexInfo(WORDINFO *wlist)
{
    FILE *fp_index = NULL;

    fp_index = fopen("/work/tan/test/my_index/index.dex","wb");
    if(fp_index == NULL)
    {
        printf("open index.dex failed\n");
        exit(1);
    }
    else
    {
        WORDINFO *p,*q;
        p=q=__INDEX_CONTEXT->wordlist;
        while(p != NULL)
        {
            fwrite(p,sizeof(WORDINFO),1,fp_index);
            printf("word=%s\n",p->word);
            p=p->next;
        }

    }

    fclose(fp_index);
  
}



void Create_Index(FILEINFO *f,char *buf)
{
    //printf("buf = %s\n",buf);
    char word[24];
    int ret=0;
    
    FILE *fp1 = fopen("/work/tan/test/my_index/dictionary.txt","r");
    
    while(fgets(word,24,fp1))
    {
        word[strlen(word)-1]=0;
      
        ret = query_dictionary(buf,word);
        if(ret)
        {
            //printf("(%s) is find!!!!! in file =%s\n",word,f->filename);
            
            UpdateWordList(f,word);
    
        }
        memset(word,0,24);
    }

    fclose(fp1);
}

int StartIndex(char *c)
{
    FILEINFO *p;
    p = __INDEX_CONTEXT->filelist;
 
	


    while(p != NULL)
    { 
        FILE *fp = fopen(p->path,"r");
        if(fp)
        {
            int c;
            char buf_sentence[MAXSENTENCELEN];
            int sentence = 0;
            int i=0;

            while (!feof(fp) && (c = fgetc(fp)) != EOF) 
            {
                if(!sentence)
                {
                    if(isblankchar(c))
                        continue;
                    else
                    {
                        i = 0;
                        buf_sentence[i++] = c;   
                        sentence = 1;
                        if (i == MAXSENTENCELEN)
                            i--;     
                    }                        
                }
                else if(sentence) 
                {
                    if(isblankchar(c))
                    { 
                    
                        buf_sentence[i++] = '\0';
                        if (i == MAXSENTENCELEN)
                            buf_sentence[--i] = '\0';
                        sentence = 0;
                        
                        if(strlen(buf_sentence)>3)
                        { 
                            query_englishword(p,buf_sentence);

                            Create_Index(p,buf_sentence);
                        }
                    }
                    else
                    {
                        buf_sentence[i++] = c;    
                        if (i == MAXSENTENCELEN)
                            i--;
                    }
                }
            }
        }
        else
            printf("open failed\n");
        
        fclose(fp);
        p = p->next;
    }
}


void CreateFileList(char *path,char *file)
{
    if(__INDEX_CONTEXT->filelist == NULL)
    {
        __INDEX_CONTEXT->filelist = (FILEINFO *)malloc(sizeof(FILEINFO));
        __INDEX_CONTEXT->filelist->filename=str_assign(file);
        __INDEX_CONTEXT->filelist->path=str_assign(path);
        __INDEX_CONTEXT->filelist->id=1;
        __INDEX_CONTEXT->filelist->next=NULL;
    }
    else
    {
        FILEINFO *p,*q,*newfile;
        p = __INDEX_CONTEXT->filelist;
        q = __INDEX_CONTEXT->filelist;
        //__INDEX_CONTEXT->count++;
        newfile = (FILEINFO *)malloc(sizeof(FILEINFO));
        
        newfile->filename = str_assign(file);
        newfile->path = str_assign(path);
        newfile->id = count_file++;
        while(p != NULL)
        { 
            q=p;
            p = p->next;
        }
        
        q->next = newfile;
        newfile->next = NULL;
    }
  
}

int IsTextFile(char *file)
{
    return 1;
}

void dir_scan(char *path, char *file) 
  { 
    struct stat s; 
    DIR *dir; 
    struct dirent *dt; 
    char dirname[50]; 
    char allpath[512];

    memset(dirname, 0, 50*sizeof(char)); 
    strcpy(dirname, path); 

    if(lstat(file, &s) < 0)
        printf("lstat error\n"); 

    if(S_ISDIR(s.st_mode))
    { 
        strcpy(dirname+strlen(dirname), file); 
        strcpy(dirname+strlen(dirname), "/"); 
        if((dir = opendir(file)) == NULL)
        { 
            printf("opendir error\n"); 
            exit(4); 
        } 
        if(chdir(file) < 0) 
        { 
            printf("chdir error\n"); 
            exit(5); 
        } 
        while((dt = readdir(dir)) != NULL)
        { 
            if(dt->d_name[0] == '.')
            { 
                continue; 
            } 
            
            dir_scan(dirname, dt->d_name); 
        } 
        if(chdir("..") < 0)
        { 
            printf("chdir error\n"); 
            exit(6); 
        } 
    }
    
    else
    {
        sprintf(allpath,"%s%s",dirname,file);  
                printf("%s\n",allpath);
        
        if(IsTextFile(allpath))
        {
            CreateFileList(allpath,file);
        }
        else
            printf("%s is not a text file\n",allpath);
    } 
  }

void IndexFiles(char * path)
{
    dir_scan("", path); 
    
    StartIndex("");
    
    if(__INDEX_CONTEXT->wordlist)
        WriteIndexInfo(__INDEX_CONTEXT->wordlist);
        //printfwordlist(__INDEX_CONTEXT->wordlist);
}


