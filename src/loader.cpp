#include <ai.h>
#include <cstring>

extern const AtNodeMethods* myOutlineMethods;
extern const AtNodeMethods* myRemapMethods;
extern const AtNodeMethods* myRampMethods;
extern const AtNodeMethods* myToonsMethods;

enum{
	MYOUTLINE = 0,
	MYREMAP,
	MYRAMP,
	MYTOONS
};

node_loader
{
	switch(i) {
	case MYOUTLINE:
		node->methods = myOutlineMethods;
		node->output_type = AI_TYPE_RGB;
		node->name = "aiMyOutlineShader";
		node->node_type = AI_NODE_SHADER;
		break;
	case MYREMAP:
		node->methods = myRemapMethods;
		node->output_type = AI_TYPE_RGB;
		node->name = "aiMyRemapShader";
		node->node_type = AI_NODE_SHADER;
		break;
	case MYRAMP:
		node->methods = myRampMethods;
		node->output_type = AI_TYPE_RGB;
		node->name = "aiMyRampShader";
		node->node_type = AI_NODE_SHADER;
		break;
	case MYTOONS:
		node->methods = myToonsMethods;
		node->output_type = AI_TYPE_RGB;
		node->name = "aiMyToonsShader";
		node->node_type = AI_NODE_SHADER;
		break;		

	default:
		return false;
	}

	strcpy(node->version, AI_VERSION);
	return true;
}
