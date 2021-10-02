#ifndef MOCKUP_FILES_H
#define MOCKUP_FILES_H

const char* file_banana[4] =
  {
   "export module banana;",
   "import orange;",
   "import mango;",
   "rest of file..."
  };

const char* file_orange[5] =
  {
   "export module orange;",
   "import mango;",
   "import walnut;",
   "export import :blood_orange;",
   "rest of file..."
  };

const char* file_blood_orange[2] =
  {
   "export module orange:blood_orange;",
   "rest of file..."
  };

const char* file_mango[4] =
  {
   "export module mango;",
   "import pineapple;",
   "import walnut;",
   "rest of file..."
  };

const char* file_pineapple[3] =
  {
   "export module pineapple;",
   "import walnut;",
   "rest of file..."
  };

const char* file_walnut[2] =
  {
   "export module walnut;",
   "rest of file..."
  };

#endif // MOCKUP_FILES_H
