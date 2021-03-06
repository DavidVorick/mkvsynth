#ifndef x264Encode_c_
#define x264Encode_c_

#include "../../jarvis/jarvis.h"
#include <stdio.h>

struct x264EncodeParams {
	char *filename;
	char *x264params;
	MkvsynthInput *input;
};

void *x264Encode(void *filterParams) {
	struct x264EncodeParams *params = (struct x264EncodeParams*)filterParams;

	char fullCommand[1024];
	
	snprintf(fullCommand, sizeof(fullCommand), "x264 - --input-csp rgb --input-depth %i --fps %i/%i --input-res %ix%i %s -o %s",
		getDepth(params->input->metaData),
		params->input->metaData->fpsNumerator,
		params->input->metaData->fpsDenominator,
		params->input->metaData->width,
		params->input->metaData->height,
		params->x264params,
		params->filename);

	FILE *x264Proc = popen(fullCommand, "w");

	MkvsynthFrame *workingFrame = getReadOnlyFrame(params->input);

	while(workingFrame->payload != NULL) {
		fwrite(workingFrame->payload, 1, getBytes(params->input->metaData), x264Proc);
		clearReadOnlyFrame(workingFrame);
		workingFrame = getReadOnlyFrame(params->input);
	}

	pclose(x264Proc);
	free(params);
	return NULL;
}

void x264Encode_AST(ASTnode *p, ASTnode *args) {
	struct x264EncodeParams *params = malloc(sizeof(struct x264EncodeParams));

	checkArgs("writeRawFile", args, 2, typeClip, typeStr);
	MkvsynthOutput *output = MANDCLIP();
	params->filename = MANDSTR();
	params->x264params = OPTSTR("params", "");
	params->input = createInputBuffer(output);

	if(isMetaDataValid(params->input->metaData) != 1) {
		printf("x264Encode Error: invalid colorspace!\n");
		exit(0);
	}

	mkvsynthQueue((void *)params, x264Encode);
}

#endif
