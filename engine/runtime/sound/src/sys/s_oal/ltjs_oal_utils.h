#ifndef LTJS_OAL_UTILS_INCLUDED
#define LTJS_OAL_UTILS_INCLUDED


namespace ltjs
{
namespace oal
{


void clear_error();

void clear_error_debug();

void ensure_no_error_debug();

#define LTJS_OAL_ENSURE_CALL_DEBUG(x) {ltjs::oal::clear_error_debug(); (x); ltjs::oal::ensure_no_error_debug();}

bool is_succeed();


} // oal
} // ltjs


#endif // !LTJS_OAL_UTILS_INCLUDED
