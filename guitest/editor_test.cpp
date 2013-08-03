#include "editor_test.h"
#include "../test/testutil.h"

#include <collabeditor.h>

void EditorTest::editTest()
{
	CollabEditor editor;
	editor.show();
	qApp->exec();
}


REGISTER_TEST(EditorTest)
