#include <mpi.h>
#include <iostream>
using namespace std;
// Получение числа процессоров по ребрам
int getNumberEnters(int numbMax){
    int count = 0;
    for(int i = 0;i < numbMax;i+=4) {
        count++;
    }
    return count;
}
int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int size_ranks;
    MPI_Comm_size(MPI_COMM_WORLD , &size_ranks);
    // Create coords
    int dims[3] = {0,0,0};
    MPI_Dims_create(size_ranks,3,dims);
    // Make both dimensions periodic
    int periods[3] = {true, true,true};
    int reorder = true;

    MPI_Comm new_communicator;

    MPI_Cart_create(MPI_COMM_WORLD,3,dims,periods,reorder,&new_communicator);
    // Get new ranks in new communicator
    int my_rank;
    MPI_Comm_rank(new_communicator,&my_rank);
    // Get coords for all proc
    int rank_coords[3];
    MPI_Cart_coords(new_communicator,my_rank,3,rank_coords);
    
    if(my_rank == 0) {
        int maxElement = dims[2] - 1;
        int starts_points[6][3];
        // Initilize starts points
        starts_points[0][0] = 0; starts_points[0][1] = 0;starts_points[0][2] = 0;
        starts_points[1][0] = 0; starts_points[1][1] = 0;starts_points[1][2] = 0;
        starts_points[2][0] = 0; starts_points[2][1] = 0;starts_points[2][2] = 0;

        starts_points[3][0] = maxElement; starts_points[3][1] = 0; starts_points[3][2] = 0;
        starts_points[4][0] = 0; starts_points[4][1] = maxElement; starts_points[4][2] = 0;
        starts_points[5][0] = 0; starts_points[5][1] = 0; starts_points[5][2] = maxElement;

        cout << " Choose side from the cube {Between 0 and 5}" << endl;
        cout << "Input user number here : ";
        int user_side;
        cin >> user_side;
        // Отправить всем процессорам выбранную сторону куба 
        for(int i = 0; i < size_ranks; i++) {
            MPI_Send(&user_side , 1 , MPI_INT , i , 0 , new_communicator);
        }
    } 
    // Считываем на какой сторонне будем вычислять  
    int maxElements = dims[2];
    int x_side;
    MPI_Recv(&x_side , 1 , MPI_INT , MPI_ANY_SOURCE , 0 , new_communicator , MPI_STATUS_IGNORE);
    // Если выбираем сторону 0 то кордината rank_coords[0] будет статичной и не будет менятся 
    if(x_side == 0 && rank_coords[0] == 0 ) {
        
        int recv_message = 0;
        int rank_source = 0;
        int rank_dest = 0;
        // Если это наш стартовый процессор с соотвествующими координатами начало то отправь следющему процессору сообщение
         if(rank_coords[1] == 0 &&  rank_coords[2] == 0) {
            cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;
            MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
            for(int i = 0;i < size_ranks; i++) {
                MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
            }
            int message = 32;
            MPI_Send(&message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
            cout << " Message: " << message << endl;
            sleep(1);
         }
        // Все процессоры проверяют если им пришло какое то сообщение  они считывают и посылают своему соседу
        for(int i = 0; i < (maxElements - 1) * 4 ; i++) {
            int recv_rank_inheriate;
            MPI_Recv(&recv_rank_inheriate , 1 , MPI_INT , MPI_ANY_SOURCE , 0 , new_communicator , MPI_STATUS_IGNORE);
        
            if(my_rank == recv_rank_inheriate) {
                // Меняем соседей если они не соотвествуют нашим требованиям от которых будем считывать сообщения
               if(rank_coords[1] < maxElements && rank_coords[2] == 0){
                    MPI_Cart_shift(new_communicator, 1, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] > 0 && rank_coords[2] < maxElements){
                    MPI_Cart_shift(new_communicator, 2, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] == maxElements - 1 && rank_coords[1] < maxElements - 1){
                    MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
                }

                 if(rank_coords[1] == 0 && rank_coords[2] <  getNumberEnters((maxElements - 1) * 4)) {
                   
                    MPI_Cart_shift(new_communicator, 2, 1, &rank_source , &rank_dest);
                }    
                cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;
                    MPI_Recv(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator , MPI_STATUS_IGNORE);
                    cout << " Get Message: " << recv_message << endl;
                sleep(1);
                // Меняем соседей если они не соотвествуют нашим требованиям которыму будем отправлять сообщения
                if(rank_coords[1] != maxElements - 1){
                    MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
                } else if(rank_coords[1] == maxElements - 1 && rank_coords[2] < maxElements ) {
                    MPI_Cart_shift(new_communicator, 2, 1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] == maxElements - 1 && rank_coords[1] != 0) {
                    MPI_Cart_shift(new_communicator, 1, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[1] == 0 && rank_coords[2] != 0) {
                    MPI_Cart_shift(new_communicator, 2, -1, &rank_source , &rank_dest);
                }

                for(int i = 0;i < size_ranks; i++) {
                    MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
                }
                    MPI_Send(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
                }
            }
    }

    // Для каждой стороны фиксируются n-ая координата и повторяется тоже самое как и для 0 только с изминением движения по координатам 

    if(x_side == 1 && rank_coords[1] == 0 ) {
        
        int recv_message = 0;
        int rank_source = 0;
        int rank_dest = 0;
        
         if(rank_coords[0] == 0 &&  rank_coords[2] == 0) {
            cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;

            MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
            
            for(int i = 0; i < size_ranks; i++) {
                MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
            }
            int message = 32;
            MPI_Send(&message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
            cout << " Message: " << message << endl;
            sleep(1);
         }

        for(int i = 0; i < (maxElements - 1) * 4 ; i++) {
            int recv_rank_inheriate;
            MPI_Recv(&recv_rank_inheriate , 1 , MPI_INT , MPI_ANY_SOURCE , 0 , new_communicator , MPI_STATUS_IGNORE);
        
            if(my_rank == recv_rank_inheriate) {
               if(rank_coords[0] < maxElements && rank_coords[2] == 0){
                    MPI_Cart_shift(new_communicator, 0, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] > 0 && rank_coords[2] < maxElements){
                    MPI_Cart_shift(new_communicator, 2, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] == maxElements - 1 && rank_coords[0] < maxElements - 1){
                    MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
                }

                 if(rank_coords[0] == 0 && rank_coords[2] <  getNumberEnters((maxElements - 1) * 4)) {
                   
                    MPI_Cart_shift(new_communicator, 2, 1, &rank_source , &rank_dest);
                }    
                cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;
                    MPI_Recv(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator , MPI_STATUS_IGNORE);
                    cout << " Message: " << recv_message << endl;
                sleep(1);
                if(rank_coords[0] != maxElements - 1){
                    MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
                } else if(rank_coords[0] == maxElements - 1 && rank_coords[2] < maxElements ) {
                    MPI_Cart_shift(new_communicator, 2, 1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] == maxElements - 1 && rank_coords[0] != 0) {
                    MPI_Cart_shift(new_communicator, 0, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[0] == 0 && rank_coords[2] != 0) {
                    MPI_Cart_shift(new_communicator, 2, -1, &rank_source , &rank_dest);
                }

                for(int i = 0;i < size_ranks; i++) {
                    MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
                }
                    MPI_Send(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
                }
            }
    }

    if(x_side == 2 && rank_coords[2] == 0) {
        
        int recv_message = 0;
        int rank_source = 0;
        int rank_dest = 0;
        
         if(rank_coords[0] == 0 &&  rank_coords[1] == 0) {
            cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;

            MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
            
            for(int i = 0;i < size_ranks; i++) {
                MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
            }
            int message = 32;
            MPI_Send(&message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
            cout << " Message: " << message << endl;
            sleep(1);
         }

        for(int i = 0; i < (maxElements - 1) * 4 ; i++) {
            int recv_rank_inheriate;
            MPI_Recv(&recv_rank_inheriate , 1 , MPI_INT , MPI_ANY_SOURCE , 0 , new_communicator , MPI_STATUS_IGNORE);
        
            if(my_rank == recv_rank_inheriate) {
               if(rank_coords[0] < maxElements && rank_coords[1] == 0){
                    MPI_Cart_shift(new_communicator, 0, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[1] > 0 && rank_coords[1] < maxElements){
                    MPI_Cart_shift(new_communicator, 1, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[1] == maxElements - 1 && rank_coords[0] < maxElements - 1){
                    MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
                }

                 if(rank_coords[0] == 0 && rank_coords[1] <  getNumberEnters((maxElements - 1) * 4)) {
                   
                    MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
                     
                }    
                cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;
                    MPI_Recv(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator , MPI_STATUS_IGNORE);
                    cout << " Message :" << recv_message << endl;
                    sleep(1);
                if(rank_coords[0] != maxElements - 1){
                    MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
                } else if(rank_coords[0] == maxElements - 1 && rank_coords[1] < maxElements ) {
                    MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
                }
                if(rank_coords[1] == maxElements - 1 && rank_coords[0] != 0) {
                    MPI_Cart_shift(new_communicator, 0, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[0] == 0 && rank_coords[1] != 0) {
                    MPI_Cart_shift(new_communicator, 1, -1, &rank_source , &rank_dest);
                }

                for(int i = 0;i < size_ranks; i++) {
                    MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
                }
                    MPI_Send(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
                }
            }
    }

    if(x_side == 3 && rank_coords[0] == maxElements - 1 ) {
       
        int recv_message = 0;
        int rank_source = 0;
        int rank_dest = 0;
        
         if(rank_coords[1] == 0 &&  rank_coords[2] == 0) {
            cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;
            MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
            for(int i = 0;i < size_ranks; i++) {
                MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
            }
            int message = 32;
            MPI_Send(&message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
            cout << " Message: " << message << endl;
            sleep(1);
         }

        for(int i = 0; i < (maxElements - 1) * 4 ; i++) {
            int recv_rank_inheriate;
            MPI_Recv(&recv_rank_inheriate , 1 , MPI_INT , MPI_ANY_SOURCE , 0 , new_communicator , MPI_STATUS_IGNORE);
        
            if(my_rank == recv_rank_inheriate) {
               if(rank_coords[1] < maxElements && rank_coords[2] == 0){
                    MPI_Cart_shift(new_communicator, 1, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] > 0 && rank_coords[2] < maxElements){
                    MPI_Cart_shift(new_communicator, 2, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] == maxElements - 1 && rank_coords[1] < maxElements - 1){
                    MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
                }

                 if(rank_coords[1] == 0 && rank_coords[2] <  getNumberEnters((maxElements - 1) * 4)) {
                   
                    MPI_Cart_shift(new_communicator, 2, 1, &rank_source , &rank_dest);
                }    
                cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;
                    MPI_Recv(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator , MPI_STATUS_IGNORE);
                    
                    cout << " Message: " << recv_message << endl;
                    sleep(1);
                if(rank_coords[1] != maxElements - 1){
                    MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
                } else if(rank_coords[1] == maxElements - 1 && rank_coords[2] < maxElements ) {
                    MPI_Cart_shift(new_communicator, 2, 1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] == maxElements - 1 && rank_coords[1] != 0) {
                    MPI_Cart_shift(new_communicator, 1, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[1] == 0 && rank_coords[2] != 0) {
                    MPI_Cart_shift(new_communicator, 2, -1, &rank_source , &rank_dest);
                }

                for(int i = 0;i < size_ranks; i++) {
                    MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
                }
                    MPI_Send(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
                }
            }
    }


    if(x_side == 4 && rank_coords[1] == maxElements - 1) {
        int recv_message = 0;
        int rank_source = 0;
        int rank_dest = 0;
        
         if(rank_coords[0] == 0 &&  rank_coords[2] == 0) {
            cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;

            MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
            
            for(int i = 0; i < size_ranks; i++) {
                MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
            }
            int message = 32;
            MPI_Send(&message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
            cout << " Message: " << message << endl;
            sleep(1);
         }

        for(int i = 0; i < (maxElements - 1) * 4 ; i++) {
            int recv_rank_inheriate;
            MPI_Recv(&recv_rank_inheriate , 1 , MPI_INT , MPI_ANY_SOURCE , 0 , new_communicator , MPI_STATUS_IGNORE);
        
            if(my_rank == recv_rank_inheriate) {
               if(rank_coords[0] < maxElements && rank_coords[2] == 0){
                    MPI_Cart_shift(new_communicator, 0, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] > 0 && rank_coords[2] < maxElements){
                    MPI_Cart_shift(new_communicator, 2, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] == maxElements - 1 && rank_coords[0] < maxElements - 1){
                    MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
                }

                 if(rank_coords[0] == 0 && rank_coords[2] <  getNumberEnters((maxElements - 1) * 4)) {
                   
                    MPI_Cart_shift(new_communicator, 2, 1, &rank_source , &rank_dest);
                }    
                cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;
                    MPI_Recv(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator , MPI_STATUS_IGNORE);
                    cout << " Message " << recv_message << endl;
                    sleep(1);
                if(rank_coords[0] != maxElements - 1){
                    MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
                } else if(rank_coords[0] == maxElements - 1 && rank_coords[2] < maxElements ) {
                    MPI_Cart_shift(new_communicator, 2, 1, &rank_source , &rank_dest);
                }
                if(rank_coords[2] == maxElements - 1 && rank_coords[0] != 0) {
                    MPI_Cart_shift(new_communicator, 0, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[0] == 0 && rank_coords[2] != 0) {
                    MPI_Cart_shift(new_communicator, 2, -1, &rank_source , &rank_dest);
                }

                for(int i = 0;i < size_ranks; i++) {
                    MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
                }
                    MPI_Send(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
                }
            }
    }

    if(x_side == 5 && rank_coords[2] == maxElements - 1) {
        
        int recv_message = 0;
        int rank_source = 0;
        int rank_dest = 0;
        
         if(rank_coords[0] == 0 &&  rank_coords[1] == 0) {
            cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;

            MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
            
            for(int i = 0;i < size_ranks; i++) {
                MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
            }
            int message = 32;
            MPI_Send(&message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
            cout << " Message: " << message << endl;
            sleep(1);
         }

        for(int i = 0; i < (maxElements - 1) * 4 ; i++) {
            int recv_rank_inheriate;
            MPI_Recv(&recv_rank_inheriate , 1 , MPI_INT , MPI_ANY_SOURCE , 0 , new_communicator , MPI_STATUS_IGNORE);
        
            if(my_rank == recv_rank_inheriate) {
               if(rank_coords[0] < maxElements && rank_coords[1] == 0){
                    MPI_Cart_shift(new_communicator, 0, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[1] > 0 && rank_coords[1] < maxElements){
                    MPI_Cart_shift(new_communicator, 1, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[1] == maxElements - 1 && rank_coords[0] < maxElements - 1){
                    MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
                }

                 if(rank_coords[0] == 0 && rank_coords[1] <  getNumberEnters((maxElements - 1) * 4)) {
                   
                    MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
                     
                }    
                cout << " ( " << my_rank << " ) " << rank_coords[0] << " " << rank_coords[1] << " " << rank_coords[2] << endl;
                    MPI_Recv(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator , MPI_STATUS_IGNORE);
                    cout << " Message : " << recv_message << endl;
                    sleep(1);
                if(rank_coords[0] != maxElements - 1){
                    MPI_Cart_shift(new_communicator, 0, 1, &rank_source , &rank_dest);
                } else if(rank_coords[0] == maxElements - 1 && rank_coords[1] < maxElements ) {
                    MPI_Cart_shift(new_communicator, 1, 1, &rank_source , &rank_dest);
                }
                if(rank_coords[1] == maxElements - 1 && rank_coords[0] != 0) {
                    MPI_Cart_shift(new_communicator, 0, -1, &rank_source , &rank_dest);
                }
                if(rank_coords[0] == 0 && rank_coords[1] != 0) {
                    MPI_Cart_shift(new_communicator, 1, -1, &rank_source , &rank_dest);
                }

                for(int i = 0;i < size_ranks; i++) {
                    MPI_Send(&rank_dest, 1 , MPI_INT , i , 0 , new_communicator );
                }
                    MPI_Send(&recv_message , 1 , MPI_INT , rank_dest , 0 , new_communicator);
                }
            }
    }

   
    MPI_Finalize(); 
    return 0;
}

