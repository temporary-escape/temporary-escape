#pragma once

#ifdef _WIN32
#ifdef SCISSIO_EXPORTS
#define SCISSIO_API __declspec(dllexport)
#else
#define SCISSIO_API __declspec(dllimport)
#endif
#elif
#define LIBRARY_API
#endif
