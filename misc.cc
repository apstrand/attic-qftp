#include <ctype.h>
#include <string.h>

int procargs(char *s, char *f[])
{
       char *p;
        int i = 0, j;
        p = s;

        while (*p) {
                f[i++] = p;
                while (*p && !isspace(*p))
                        p++;
                if (!*p)
                        break;
                *p++ = 0;
                while (*p && isspace(*p))
                        p++;
	}
        *p++ = 0;
        *p = 0;
        f[i] = 0;
	for (j = i;j < 100;j++)
		f[j] = 0;
        return i;
}


char *split(char *cwd, char *s, char *path)
{
	char *p, *p2, *p3;
	if (s)
		p = s;
	else
		p = "";
	if (*p == '/') {
		if (!(p2 = strrchr(&p[1], '/'))) {
			p2 = &p[1];
			p3 = p2;
		} else
			p3 = p2 + 1;
			
		strncpy(path, p, p2 - p);
		path[p2 - p] = 0;
		p = p3;
	} else {
		if (!(p2 = strrchr(p, '/'))) {
			strcpy(path, cwd);
		} else {
			strcpy(path, cwd);
			p3 = path + strlen(path);
			*p3++  = '/';
			strncpy(p3, p, p2 - p);
			p3[p2 - p] = 0;
			p = p2 + 1;
		}
	}
	return p;
}	
