#include "intercepter.h"

Intercepter* Intercepter::intercepter_=nullptr;
std::mutex Intercepter::mutex_;

Intercepter* Intercepter::GetInstance(Ui::MainWindow * ui){

    std::lock_guard<std::mutex> lock(mutex_);
    if(intercepter_==nullptr){
        intercepter_=new Intercepter(ui);
    }
    return intercepter_;
}





