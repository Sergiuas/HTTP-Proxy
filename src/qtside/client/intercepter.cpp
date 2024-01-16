#include "intercepter.h"

Intercepter* Intercepter::intercepter_=nullptr;
std::mutex Intercepter::mutex_;

Intercepter* Intercepter::GetInstance(){

    std::lock_guard<std::mutex> lock(mutex_);
    if(intercepter_==nullptr){
        intercepter_=new Intercepter();
    }
    return intercepter_;
}
