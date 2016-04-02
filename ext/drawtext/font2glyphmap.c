#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "drawtext.h"

#define SUFFIX	"glyphmap"
#define DEF_SIZE	12

struct coderange {
	int start, end;
	struct coderange *next;
};

void print_usage(const char *argv0);
int font2glyphmap(struct dtx_font *font, const char *infname, const char *outfname, int size, int rstart, int rend);

int main(int argc, char **argv)
{
	int i, font_size = DEF_SIZE, suffix_len = strlen(SUFFIX);
	struct coderange *clist = 0;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-range") == 0) {
				struct coderange *node;
				int start, end;

				if(sscanf(argv[++i], "%i-%i", &start, &end) != 2) {
					fprintf(stderr, "-range must be followed by a range of the form: START-END\n");
					return 1;
				}

				if(!(node = malloc(sizeof *node))) {
					perror("failed to allocate memory");
					return 1;
				}
				node->start = start;
				node->end = end;
				node->next = clist;
				clist = node;
			} else if(strcmp(argv[i], "-size") == 0) {
				char *endp;

				font_size = strtol(argv[++i], &endp, 10);
				if(endp == argv[i]) {
					fprintf(stderr, "-size must be followed by the font size\n");
					return 1;
				}
			} else {
				if(strcmp(argv[i], "-help") != 0 && strcmp(argv[i], "-h") != 0) {
					fprintf(stderr, "invalid option: %s\n", argv[i]);
				}
				print_usage(argv[0]);
				return 1;
			}
		} else {
			char *basename, *dotptr, *outfile, *lastslash;
			struct dtx_font *font;

			if(!(font = dtx_open_font(argv[i], clist ? 0 : font_size))) {
				fprintf(stderr, "failed to open font file: %s\n", argv[i]);
				return -1;
			}

			basename = alloca(strlen(argv[i]) + suffix_len + 1);
			strcpy(basename, argv[i]);

			if((dotptr = strrchr(basename, '.'))) {
				*dotptr = 0;
			}
			if((lastslash = strrchr(basename, '/'))) {
				basename = lastslash + 1;
			}

			outfile = alloca(strlen(basename) + 64);

			if(clist) {
				while(clist) {
					struct coderange *r = clist;
					clist = clist->next;

					sprintf(outfile, "%s_s%d_r%04x-%04x.%s", basename, font_size, r->start, r->end, SUFFIX);
					font2glyphmap(font, argv[i], outfile, font_size, r->start, r->end);

					free(r);
				}
				clist = 0;
			} else {
				sprintf(outfile, "%s_s%d.%s", basename, font_size, SUFFIX);
				font2glyphmap(font, argv[i], outfile, font_size, -1, -1);
			}
		}
	}

	return 0;
}

void print_usage(const char *argv0)
{
	printf("usage: %s [options] <font-1> [<font-2> ... <font-n>]\n", argv0);
	printf("options:\n");
	printf("  -size <pt>: point size (default: %d)\n", DEF_SIZE);
	printf("  -range <low>-<high>: unicode range (default: ascii)\n");
	printf("  -help: print usage information and exit\n");
}

int font2glyphmap(struct dtx_font *font, const char *infname, const char *outfname, int size, int rstart, int rend)
{
	struct dtx_glyphmap *gmap;

	if(rstart != -1) {
		dtx_prepare_range(font, size, rstart, rend);
		if(!(gmap = dtx_get_font_glyphmap(font, size, rstart))) {
			fprintf(stderr, "failed to retrieve unicode glyphmap (code range: %d-%d)\n", rstart, rend);
			return -1;
		}
	} else {
		dtx_prepare(font, size);
		if(!(gmap = dtx_get_font_glyphmap(font, size, ' '))) {
			fprintf(stderr, "failed to retrieve ASCII glyphmap!\n");
			return -1;
		}
	}

	if(dtx_save_glyphmap(outfname, gmap) == -1) {
		fprintf(stderr, "failed to save glyphmap to: %s\n", outfname);
		return -1;
	}

	return 0;
}
