#include "intercepter.h"

Intercepter* Intercepter::intercepter_=nullptr;
std::mutex Intercepter::mutex_;



Intercepter* Intercepter::GetInstance(){
    if(intercepter_==nullptr){
        intercepter_=new Intercepter();
    }
    return intercepter_;
}

