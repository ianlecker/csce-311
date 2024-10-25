//include<semaphore.h>
#include<iostream>
#include<vector>
#include<pthread.h>

#define MAX_N 100000 // 0,1,..MAX_N-1 is MAX_N numbers

struct shared{
    int n_threads;//used for stride
    std::vector<int> even_or_odd;
};

struct targs{
    shared* sh;
    int offset;
};

void* eo_logic(void* args){
    targs* ta = (targs*) args;
    int curr_i =ta->offset;
    int stride = ta->sh->n_threads;
    int* calcs = new int;
    *calcs=0;
    for( ; curr_i<MAX_N; curr_i+=stride){
        // if (curr_i ==5){
        //     ta->sh->even_or_odd[curr_i] =0; //wrong, to test
        //     *calcs=*calcs+1;
        //     continue;
        // }
        ta->sh->even_or_odd[curr_i] = curr_i % 2;
        *calcs=*calcs+1;
    }

    return calcs;
}

int main(int argc, char* argv[]){
    using std::cout;
    using std::endl;
    
    int n_threads = 4;
    if(argc==2) {
        n_threads = atoi(argv[1]);
        cout<<"Using "<<n_threads<<" threads."<<endl;
    } else{
        cout<<"Defaulting to "<<n_threads<<" threads."<<endl;
    }

    shared sh;
    sh.n_threads = n_threads;
    for(int i =0;i<MAX_N;++i){
        sh.even_or_odd.push_back(-1);
    }

    cout<<"sh->even_or_odd = [ ";
    for(int i=0;i<MAX_N;++i){ 
        cout<<sh.even_or_odd[i]<< (i+1==MAX_N?" ":", ") ;
    }
    cout<<"], initially"<<endl;

    pthread_t threads[n_threads];
    targs     all_targs[n_threads];
    for(int i =0;i<n_threads;++i){
        all_targs[i].sh=&sh;
        all_targs[i].offset = i;
        pthread_create(&threads[i],NULL,eo_logic,&all_targs[i]);
    }

    int* ret;
    int total_calcs=0;
    for(int i =0;i<n_threads;++i){
        pthread_join(threads[i],(void**) &ret);
        total_calcs+= *ret;
        delete ret;
    }

    bool all_good = true;
    cout<<"sh->even_or_odd = [ ";
    for(int i=0;i<MAX_N;++i){ 
        cout<<sh.even_or_odd[i]<< (i+1==MAX_N?" ":", ") ;
        if(sh.even_or_odd[i] != i%2) {
            all_good = false;
        }
    }
    cout<<"]"<<endl;

    cout<<"There were " << total_calcs <<" calculations spread accross "
        <<n_threads<<" threads"<<endl;
    cout<<"(there should be "<<MAX_N<< " total calculations)"<<endl;

    if(!all_good){
        cout<<"There's an error somewhere...."<<endl;
        return 1;
    }

    cout<<"Everything looks good"<<endl;
    return 0;
}