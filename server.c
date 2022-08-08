#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>


//USER,TRAIN, BOOKING STRUCTURES//
struct booking{
 int booking_id;
 int user_type;
 int user_id;
 int train_id;
 int seats;
};
struct train{
 int train_number;
 char train_name[50];
 int total_seats;
 int available_seats;
		};
struct user{
 int login_id;
 char password[50];
 char user_name[50];
 int user_type;
		};

void client_fun(int socket);
void sign_in(int client_sd);
void signup(int client_sd);
int menu(int client_sd,int user_type,int id);
void trainOperations(int client_sd);
void userOperations(int client_sd);
int book_tkt(int client_sd,int ch,int user_type,int id);

int main(void) {
 
    int sd, client_sd, sockaddr_size; 
    struct sockaddr_in server, client; 
    char buf[100]; 
  
    sd = socket(AF_INET, SOCK_STREAM, 0); 
    perror("socket : ");
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8000); 
   
    bind(sd, (struct sockaddr*)&server, sizeof(server));
    perror("bind : "); 
   
 
    listen(sd, 3);  
    sockaddr_size = sizeof(struct sockaddr_in); 
  
    while (1){

	    client_sd = accept(sd, (struct sockaddr*)&client, (socklen_t*)&sockaddr_size); 
	  
	    if (!fork()){
		    close(sd);
		    client_fun(client_sd);								// Service clients
		    exit(0);
	    }
	    else
	    	close(client_sd);
    }
    return 0;
}


//Function to service clients//
void client_fun(int socket){
	int ch;
	printf("\n\tClient [%d] Connected\n", socket);
	do{
		read(socket, &ch, sizeof(int));		
		if(ch==1)
			sign_in(socket);
		if(ch==2)
			signup(socket);
		if(ch==3)
			break;
	}while(1);

	close(socket);
	printf("\n\tClient [%d] Disconnected\n", socket);
}

//Sign-in function//

void sign_in(int client_sd){
	int u_fd = open("db/db_user",O_RDWR);
	int u_id,user_type,valid_pass=0,valid_user=0;
	char user_pass[50];
	struct user u;
	read(client_sd,&u_id,sizeof(u_id));
	read(client_sd,&user_pass,sizeof(user_pass));
	
	struct flock lock;
	
	lock.l_start = (u_id-1)*sizeof(u);
	lock.l_len = sizeof(u);
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();

	lock.l_type = F_WRLCK;
	fcntl(u_fd,F_SETLKW, &lock);
	
	while(read(u_fd,&u,sizeof(u))){
		if(u.login_id==u_id){
			valid_user=1;
			if(!strcmp(u.password,user_pass)){
				valid_pass = 1;
				user_type = u.user_type;
				break;
			}
			else{
				valid_pass = 0;
				break;
			}	
		}		
		else{
			valid_user = 0;
			valid_pass = 0;
		}
	}
	
	// same agent is allowed from multiple terminals.. 
	
	if(user_type!=2){
		lock.l_type = F_UNLCK;
		fcntl(u_fd, F_SETLK, &lock);
		close(u_fd);
	}
	
	
	if(valid_user)
	{
		write(client_sd,&valid_pass,sizeof(valid_pass));
		if(valid_pass){
			write(client_sd,&user_type,sizeof(user_type));
			while(menu(client_sd,user_type,u_id)!=-1);
		}
	}
	else
		write(client_sd,&valid_pass,sizeof(valid_pass));
	
	// same customer is not allowed from multiple terminals.. 

	if(user_type==2){
		lock.l_type = F_UNLCK;
		fcntl(u_fd, F_SETLK, &lock);
		close(u_fd);
	}
} 

// Register function//

void signup(int client_sd){
	int u_fd = open("db/db_user",O_RDWR);
	int user_type,u_id=0;
	char user_name[50],user_pass[50];
	struct user u,temp;

	read(client_sd, &user_type, sizeof(user_type));
	read(client_sd, &user_name, sizeof(user_name));
	read(client_sd, &user_pass, sizeof(user_pass));

	int fp = lseek(u_fd, 0, SEEK_END);

	struct flock lock;
	lock.l_type = F_WRLCK;
	lock.l_start = fp;
	lock.l_len = 0;
	lock.l_whence = SEEK_SET;
	lock.l_pid = getpid();


	fcntl(u_fd,F_SETLKW, &lock);

	// if file is empty, login id starts from 1
	
	if(fp==0){
		u.login_id = 1;
		strcpy(u.user_name, user_name);
		strcpy(u.password, user_pass);
		u.user_type=user_type;
		write(u_fd, &u, sizeof(u));
		write(client_sd, &u.login_id, sizeof(u.login_id));
	}
	// else it will increment from the previous value
	else{
		fp = lseek(u_fd, -1 * sizeof(u), SEEK_END);
		read(u_fd, &u, sizeof(u));
		u.login_id++;
		strcpy(u.user_name, user_name);
		strcpy(u.password, user_pass);
		u.user_type=user_type;
		write(u_fd, &u, sizeof(u));
		write(client_sd, &u.login_id, sizeof(u.login_id));
	}
	lock.l_type = F_UNLCK;
	fcntl(u_fd, F_SETLK, &lock);

	close(u_fd);
	
}

// Main menu function//

int menu(int client_sock,int user_type,int id){
	int ch,ret;

	// for admin
	if(user_type==0){
		read(client_sock,&ch,sizeof(ch));
		if(ch==1){					// train operations
			trainOperations(client_sock);
			return menu(client_sock,user_type,id);	
		}
		else if(ch==2){				// user operations
			userOperations(client_sock);
			return menu(client_sock,user_type,id);
		}
		else if (ch ==3)				// Logout
			return -1;
	}
	else if(user_type==2 || user_type==1){				// agent and customer
		read(client_sock,&ch,sizeof(ch));
		ret = book_tkt(client_sock,ch,user_type,id);
		if(ret!=5)
			return menu(client_sock,user_type,id);
		else if(ret==5)
			return -1;
	}		
}




// operations on user//
void userOperations(int client_sock){
	int flag1=0;	
	int ch;
	read(client_sock,&ch,sizeof(ch));
	if(ch==1){    					// Add user
		char user_name[50],password[50];
		int user_type;
		read(client_sock, &user_type, sizeof(user_type));
		read(client_sock, &user_name, sizeof(user_name));
		read(client_sock, &password, sizeof(password));
		
		struct user user_db;
		struct flock lock;
		int u_fd = open("db/db_user", O_RDWR);
		int fp = lseek(u_fd, 0, SEEK_END);
		
		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(u_fd, F_SETLKW, &lock);

		if(fp==0){
			user_db.login_id = 1;
			strcpy(user_db.user_name, user_name);
			strcpy(user_db.password, password);
			user_db.user_type=user_type;
			write(u_fd, &user_db, sizeof(user_db));
			flag1 = 1;
			write(client_sock,&flag1,sizeof(int));
			write(client_sock, &user_db.login_id, sizeof(user_db.login_id));
		}
		else{
			fp = lseek(u_fd, -1 * sizeof(struct user), SEEK_END);
			read(u_fd, &user_db, sizeof(user_db));
			user_db.login_id++;
			strcpy(user_db.user_name, user_name);
			strcpy(user_db.password, password);
			user_db.user_type=user_type;
			write(u_fd, &user_db, sizeof(user_db));
			flag1 = 1;
			write(client_sock,&flag1,sizeof(int));
			write(client_sock, &user_db.login_id, sizeof(user_db.login_id));
		}
		lock.l_type = F_UNLCK;
		fcntl(u_fd, F_SETLK, &lock);
		close(u_fd);
		
	}
    
	else if(ch==2){					// View user list
		struct flock lock;
		struct user user_db;
		int u_fd = open("db/db_user", O_RDONLY);
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(u_fd, F_SETLKW, &lock);
		int fp = lseek(u_fd, 0, SEEK_END);
		int no_of_users = fp / sizeof(struct user);
		no_of_users--;
		write(client_sock, &no_of_users, sizeof(int));

		lseek(u_fd,0,SEEK_SET);
		while(fp != lseek(u_fd,0,SEEK_CUR)){
			read(u_fd,&user_db,sizeof(user_db));
			if(user_db.user_type!=0){
				write(client_sock,&user_db.login_id,sizeof(int));
				write(client_sock,&user_db.user_name,sizeof(user_db.user_name));
				write(client_sock,&user_db.user_type,sizeof(int));
			}
		}
		flag1 = 1;
		lock.l_type = F_UNLCK;
		fcntl(u_fd, F_SETLK, &lock);
		close(u_fd);
	}

	else if(ch==3){					// Update user
		userOperations(client_sock);
		int ch,flag1=0,user_id;
		char pass[50];
		struct flock lock;
		struct user user_db;
		int u_fd = open("db/db_user", O_RDWR);

		read(client_sock,&user_id,sizeof(user_id));

		lock.l_type = F_WRLCK;
		lock.l_start =  (user_id-1)*sizeof(struct user);
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(u_fd, F_SETLKW, &lock);

		lseek(u_fd, 0, SEEK_SET);
		lseek(u_fd, (user_id-1)*sizeof(struct user), SEEK_CUR);
		read(u_fd, &user_db, sizeof(struct user));
		
		read(client_sock,&ch,sizeof(int));
		if(ch==1){					// update user name
			write(client_sock,&user_db.user_name,sizeof(user_db.user_name));
			read(client_sock,&user_db.user_name,sizeof(user_db.user_name));
			flag1=1;
			write(client_sock,&flag1,sizeof(flag1));		
		}
		else if(ch==2){				// update password
			read(client_sock,&pass,sizeof(pass));
			if(!strcmp(user_db.password,pass))
				flag1 = 1;
			write(client_sock,&flag1,sizeof(flag1));
			read(client_sock,&user_db.password,sizeof(user_db.password));
		}
	
		lseek(u_fd, -1*sizeof(struct user), SEEK_CUR);
		write(u_fd, &user_db, sizeof(struct user));
		if(flag1)
			write(client_sock,&flag1,sizeof(flag1));
		lock.l_type = F_UNLCK;
		fcntl(u_fd, F_SETLK, &lock);
		close(u_fd);	
	}

	else if(ch==4){						// Delete an user
		userOperations(client_sock);
		struct flock lock;
		struct user user_db;
		int u_fd = open("db/db_user", O_RDWR);
		int user_id,flag1=0;

		read(client_sock,&user_id,sizeof(user_id));

		lock.l_type = F_WRLCK;
		lock.l_start =  (user_id-1)*sizeof(struct user);
		lock.l_len = sizeof(struct user);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(u_fd, F_SETLKW, &lock);
		
		lseek(u_fd, 0, SEEK_SET);
		lseek(u_fd, (user_id-1)*sizeof(struct user), SEEK_CUR);
		read(u_fd, &user_db, sizeof(struct user));
		strcpy(user_db.user_name,"deleted");
		strcpy(user_db.password,"");
		lseek(u_fd, -1*sizeof(struct user), SEEK_CUR);
		write(u_fd, &user_db, sizeof(struct user));
		flag1=1;
		write(client_sock,&flag1,sizeof(flag1));
		lock.l_type = F_UNLCK;
		fcntl(u_fd, F_SETLK, &lock);
		close(u_fd);	
	}
	    else if(ch==5){					// Search for an user
		struct flock lock;
		struct user user_db;
		int u_fd = open("db/db_user", O_RDONLY);
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(u_fd, F_SETLKW, &lock);
		int fp = lseek(u_fd, 0, SEEK_END);
		int no_of_users = fp / sizeof(struct user);
		no_of_users--;
		write(client_sock, &no_of_users, sizeof(int));

		lseek(u_fd,0,SEEK_SET);
		while(fp != lseek(u_fd,0,SEEK_CUR)){
			read(u_fd,&user_db,sizeof(user_db));
			if(user_db.user_type!=0){
				write(client_sock,&user_db.login_id,sizeof(int));
				write(client_sock,&user_db.user_name,sizeof(user_db.user_name));
				write(client_sock,&user_db.user_type,sizeof(int));
			}
		}
		flag1 = 1;
		lock.l_type = F_UNLCK;
		fcntl(u_fd, F_SETLK, &lock);
		close(u_fd);
	}
	

}
void trainOperations(int client_sock){
	int flag1=0;	
	int ch;
	read(client_sock,&ch,sizeof(ch));
	if(ch==1){  					// Add train  	
		char tname[50];
		int train_id = 0;
		read(client_sock,&tname,sizeof(tname));
		struct train train_db,temp;
		struct flock lock;
		int t_fd = open("db/db_train", O_RDWR);
		
		train_db.train_number = train_id;
		strcpy(train_db.train_name,tname);
		train_db.total_seats = 100;				
		train_db.available_seats = 100;

		int fp = lseek(t_fd, 0, SEEK_END); 

		lock.l_type = F_WRLCK;
		lock.l_start = fp;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();

		fcntl(t_fd, F_SETLKW, &lock);

		if(fp == 0){
			flag1 = 1;
			write(t_fd, &train_db, sizeof(train_db));
			lock.l_type = F_UNLCK;
			fcntl(t_fd, F_SETLK, &lock);
			close(t_fd);
			write(client_sock, &flag1, sizeof(flag1));
		}
		else{
			flag1 = 1;
			lseek(t_fd, -1 * sizeof(struct train), SEEK_END);
			read(t_fd, &temp, sizeof(temp));
			train_db.train_number = temp.train_number + 1;
			write(t_fd, &train_db, sizeof(train_db));
			write(client_sock, &flag1,sizeof(flag1));	
		}
		lock.l_type = F_UNLCK;
		fcntl(t_fd, F_SETLK, &lock);
		close(t_fd);
		
	}
	else if(ch==2){					// Search trains
		struct flock lock;
		struct train train_db;
		int t_fd = open("db/db_train", O_RDONLY);
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(t_fd, F_SETLKW, &lock);
		int fp = lseek(t_fd, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);
		write(client_sock, &no_of_trains, sizeof(int));

		lseek(t_fd,0,SEEK_SET);
		while(fp != lseek(t_fd,0,SEEK_CUR)){
			read(t_fd,&train_db,sizeof(train_db));
			write(client_sock,&train_db.train_number,sizeof(int));
			write(client_sock,&train_db.train_name,sizeof(train_db.train_name));
			write(client_sock,&train_db.total_seats,sizeof(int));
			write(client_sock,&train_db.available_seats,sizeof(int));
		}
		flag1 = 1;
		lock.l_type = F_UNLCK;
		fcntl(t_fd, F_SETLK, &lock);
		close(t_fd);
	}

	else if(ch==3){					// View list of trains
		struct flock lock;
		struct train train_db;
		int t_fd = open("db/db_train", O_RDONLY);
		
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(t_fd, F_SETLKW, &lock);
		int fp = lseek(t_fd, 0, SEEK_END);
		int no_of_trains = fp / sizeof(struct train);
		write(client_sock, &no_of_trains, sizeof(int));

		lseek(t_fd,0,SEEK_SET);
		while(fp != lseek(t_fd,0,SEEK_CUR)){
			read(t_fd,&train_db,sizeof(train_db));
			write(client_sock,&train_db.train_number,sizeof(int));
			write(client_sock,&train_db.train_name,sizeof(train_db.train_name));
			write(client_sock,&train_db.total_seats,sizeof(int));
			write(client_sock,&train_db.available_seats,sizeof(int));
		}
		flag1 = 1;
		lock.l_type = F_UNLCK;
		fcntl(t_fd, F_SETLK, &lock);
		close(t_fd);
	}

	else if(ch==4){					// Update train
		trainOperations(client_sock);
		int ch,flag1=0,train_id;
		int val,old_val;
		struct flock lock;
		struct train train_db;
		int t_fd = open("db/db_train", O_RDWR);

		read(client_sock,&train_id,sizeof(train_id));

		lock.l_type = F_WRLCK;
		lock.l_start = (train_id)*sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(t_fd, F_SETLKW, &lock);

		lseek(t_fd, 0, SEEK_SET);
		lseek(t_fd, (train_id)*sizeof(struct train), SEEK_CUR);
		read(t_fd, &train_db, sizeof(struct train));
		
		read(client_sock,&ch,sizeof(int));
		if(ch==1){							// update train user_name
			write(client_sock,&train_db.train_name,sizeof(train_db.train_name));
			read(client_sock,&train_db.train_name,sizeof(train_db.train_name));
			
		}
		else if(ch==2){	
		       		
		        			// update total number of seats
		       read(client_sock,&val,sizeof(val));
		       train_db.total_seats = train_db.total_seats + val;
		       train_db.available_seats = train_db.available_seats + val;
		}
		
		
		
	
		lseek(t_fd, -1*sizeof(struct train), SEEK_CUR);
		write(t_fd, &train_db, sizeof(struct train));
		flag1=1;
		write(client_sock,&flag1,sizeof(flag1));
		lock.l_type = F_UNLCK;
		fcntl(t_fd, F_SETLK, &lock);
		close(t_fd);	
	}

	else if(ch==5){						// Delete train
		trainOperations(client_sock);
		struct flock lock;
		struct train train_db;
		int t_fd = open("db/db_train", O_RDWR);
		int train_id,flag1=0;

		read(client_sock,&train_id,sizeof(train_id));

		lock.l_type = F_WRLCK;
		lock.l_start = (train_id)*sizeof(struct train);
		lock.l_len = sizeof(struct train);
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(t_fd, F_SETLKW, &lock);
		
		lseek(t_fd, 0, SEEK_SET);
		lseek(t_fd, (train_id)*sizeof(struct train), SEEK_CUR);
		read(t_fd, &train_db, sizeof(struct train));
		strcpy(train_db.train_name,"deleted");
		lseek(t_fd, -1*sizeof(struct train), SEEK_CUR);
		write(t_fd, &train_db, sizeof(struct train));
		flag1=1;
		write(client_sock,&flag1,sizeof(flag1));
		lock.l_type = F_UNLCK;
		fcntl(t_fd, F_SETLK, &lock);
		close(t_fd);	
	}
	
	
}


// TICKET BOOKING //
int book_tkt(int client_sock,int ch,int user_type,int id){
	int flag1=0;
	if(ch==1){						// book ticket
		trainOperations(client_sock);
		struct flock lock_train;
		struct flock lock_booking;
		struct train train_db;
		struct booking booking_db;
		int t_fd = open("db/db_train", O_RDWR);
		int b_fd = open("db/db_booking", O_RDWR);
		int train_id,seats;
		read(client_sock,&train_id,sizeof(train_id));		
				
		lock_train.l_type = F_WRLCK;
		lock_train.l_start = train_id*sizeof(struct train);
		lock_train.l_len = sizeof(struct train);
		lock_train.l_whence = SEEK_SET;
		lock_train.l_pid = getpid();
		
		lock_booking.l_type = F_WRLCK;
		lock_booking.l_start = 0;
		lock_booking.l_len = 0;
		lock_booking.l_whence = SEEK_END;
		lock_booking.l_pid = getpid();
		
		fcntl(t_fd, F_SETLKW, &lock_train);
		lseek(t_fd,train_id*sizeof(struct train),SEEK_SET);
		
		read(t_fd,&train_db,sizeof(train_db));
		read(client_sock,&seats,sizeof(seats));

		if(train_db.train_number==train_id)
		{		
			if(train_db.available_seats>=seats){
				flag1 = 1;
				train_db.available_seats -= seats;
				fcntl(b_fd, F_SETLKW, &lock_booking);
				int fp = lseek(b_fd, 0, SEEK_END);
				
				if(fp > 0){
					lseek(b_fd, -1*sizeof(struct booking), SEEK_CUR);
					read(b_fd, &booking_db, sizeof(struct booking));
					booking_db.booking_id++;
				}
				else 
					booking_db.booking_id = 0;

				booking_db.user_type = user_type;
				booking_db.user_id = id;
				booking_db.train_id = train_id;
				booking_db.seats = seats;
				write(b_fd, &booking_db, sizeof(struct booking));
				lock_booking.l_type = F_UNLCK;
				fcntl(b_fd, F_SETLK, &lock_booking);
			 	close(b_fd);
			}
		
		lseek(t_fd, -1*sizeof(struct train), SEEK_CUR);
		write(t_fd, &train_db, sizeof(train_db));
		}

		lock_train.l_type = F_UNLCK;
		fcntl(t_fd, F_SETLK, &lock_train);
		close(t_fd);
		write(client_sock,&flag1,sizeof(flag1));
		return flag1;		
	}
	
	else if(ch==2){							// View bookings
		struct flock lock;
		struct booking booking_db;
		int b_fd = open("db/db_booking", O_RDONLY);
		int no_of_bookings = 0;
	
		lock.l_type = F_RDLCK;
		lock.l_start = 0;
		lock.l_len = 0;
		lock.l_whence = SEEK_SET;
		lock.l_pid = getpid();
		
		fcntl(b_fd, F_SETLKW, &lock);
	
		while(read(b_fd,&booking_db,sizeof(booking_db))){
			if (booking_db.user_id==id)
				no_of_bookings++;
		}

		write(client_sock, &no_of_bookings, sizeof(int));
		lseek(b_fd,0,SEEK_SET);

		while(read(b_fd,&booking_db,sizeof(booking_db))){
			if(booking_db.user_id==id){
				write(client_sock,&booking_db.booking_id,sizeof(int));
				write(client_sock,&booking_db.train_id,sizeof(int));
				write(client_sock,&booking_db.seats,sizeof(int));
			}
		}
		lock.l_type = F_UNLCK;
		fcntl(b_fd, F_SETLK, &lock);
		close(b_fd);
		return flag1;
	}

	else if (ch==3){							// update booking
		int ch = 2,booking_id,val;
		book_tkt(client_sock,ch,user_type,id);
		struct booking booking_db;
		struct train train_db;
		struct flock lock_booking;
		struct flock lock_train;
		int b_fd = open("db/db_booking", O_RDWR);
		int t_fd = open("db/db_train", O_RDWR);
		read(client_sock,&booking_id,sizeof(booking_id));

		lock_booking.l_type = F_WRLCK;
		lock_booking.l_start = booking_id*sizeof(struct booking);
		lock_booking.l_len = sizeof(struct booking);
		lock_booking.l_whence = SEEK_SET;
		lock_booking.l_pid = getpid();
		
		fcntl(b_fd, F_SETLKW, &lock_booking);
		lseek(b_fd,booking_id*sizeof(struct booking),SEEK_SET);
		read(b_fd,&booking_db,sizeof(booking_db));
		lseek(b_fd,-1*sizeof(struct booking),SEEK_CUR);
		
		lock_train.l_type = F_WRLCK;
		lock_train.l_start = (booking_db.train_id)*sizeof(struct train);
		lock_train.l_len = sizeof(struct train);
		lock_train.l_whence = SEEK_SET;
		lock_train.l_pid = getpid();

		fcntl(t_fd, F_SETLKW, &lock_train);
		lseek(t_fd,(booking_db.train_id)*sizeof(struct train),SEEK_SET);
		read(t_fd,&train_db,sizeof(train_db));
		lseek(t_fd,-1*sizeof(struct train),SEEK_CUR);

		read(client_sock,&ch,sizeof(ch));
	
		if(ch==1){							// increase number of seats 
			read(client_sock,&val,sizeof(val));
			if(train_db.available_seats>=val){
				flag1=1;
				train_db.available_seats -= val;
				booking_db.seats += val;
			}
		}
		else if(ch==2){						// decrease number of seats
			flag1=1;
			read(client_sock,&val,sizeof(val));
			train_db.available_seats += val;
			booking_db.seats -= val;	
		}
		
		write(t_fd,&train_db,sizeof(train_db));
		lock_train.l_type = F_UNLCK;
		fcntl(t_fd, F_SETLK, &lock_train);
		close(t_fd);
		
		write(b_fd,&booking_db,sizeof(booking_db));
		lock_booking.l_type = F_UNLCK;
		fcntl(b_fd, F_SETLK, &lock_booking);
		close(b_fd);
		
		write(client_sock,&flag1,sizeof(flag1));
		return flag1;
	}
	else if(ch==4){							// Cancel a booking
		int ch = 2,booking_id;
		book_tkt(client_sock,ch,user_type,id);
		struct booking booking_db;
		struct train train_db;
		struct flock lock_booking;
		struct flock lock_train;
		int b_fd = open("db/db_booking", O_RDWR);
		int t_fd = open("db/db_train", O_RDWR);
		read(client_sock,&booking_id,sizeof(booking_id));

		lock_booking.l_type = F_WRLCK;
		lock_booking.l_start = booking_id*sizeof(struct booking);
		lock_booking.l_len = sizeof(struct booking);
		lock_booking.l_whence = SEEK_SET;
		lock_booking.l_pid = getpid();
		
		fcntl(b_fd, F_SETLKW, &lock_booking);
		lseek(b_fd,booking_id*sizeof(struct booking),SEEK_SET);
		read(b_fd,&booking_db,sizeof(booking_db));
		lseek(b_fd,-1*sizeof(struct booking),SEEK_CUR);
		
		lock_train.l_type = F_WRLCK;
		lock_train.l_start = (booking_db.train_id)*sizeof(struct train);
		lock_train.l_len = sizeof(struct train);
		lock_train.l_whence = SEEK_SET;
		lock_train.l_pid = getpid();

		fcntl(t_fd, F_SETLKW, &lock_train);
		lseek(t_fd,(booking_db.train_id)*sizeof(struct train),SEEK_SET);
		read(t_fd,&train_db,sizeof(train_db));
		lseek(t_fd,-1*sizeof(struct train),SEEK_CUR);

		train_db.available_seats += booking_db.seats;
		booking_db.seats = 0;
		flag1 = 1;

		write(t_fd,&train_db,sizeof(train_db));
		lock_train.l_type = F_UNLCK;
		fcntl(t_fd, F_SETLK, &lock_train);
		close(t_fd);
		
		write(b_fd,&booking_db,sizeof(booking_db));
		lock_booking.l_type = F_UNLCK;
		fcntl(b_fd, F_SETLK, &lock_booking);
		close(b_fd);
		
		write(client_sock,&flag1,sizeof(flag1));
		return flag1;
		
	}
	
	else if(ch==5)										// Logout
		return 5;


}

