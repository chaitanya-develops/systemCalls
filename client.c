#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 

int menu(int ser_socket,int user_type);
int book_tkt(int ser_socket,int ch);
int trainOperations(int ser_socket,int ch);
int userOperations(int ser_socket,int ch);

int client(int ser_socket){
	int ch,flag1;
	printf("\n\n==========  Welcome to train ticket reservation system  ==========\n\n");
	printf("(1) Sign In\n\n");
	printf("(2) Register\n\n");
	printf("(3) Exit\n\n");
	printf("\n Choose a valid option : ");
	scanf("%d", &ch);
	write(ser_socket, &ch, sizeof(ch));
	if (ch == 1){					// sign in
		int user_id,user_type;
		char user_pass[10];
		printf("\tEnter your sign in id to proceed : ");
		scanf("%d", &user_id);
		strcpy(user_pass,getpass("\tEnter your password : "));
		write(ser_socket, &user_id, sizeof(user_id));
		write(ser_socket, &user_pass, sizeof(user_pass));
		read(ser_socket, &flag1, sizeof(flag1));
		if(flag1){
			printf("\tYou have sucessfully logged in\n");
			read(ser_socket,&user_type,sizeof(user_type));
			while(menu(ser_socket,user_type)!=-1);
			return 1;
		}
		else{
			printf("\tYour login failed : Incorrect password or login id\n");
			return 1;
		}
	}

	else if(ch == 2){					// Register
		int user_type,user_id;
		char user_name[50],user_pass[10];
		system("clear");
		printf("\n\tChoose the type of account:: \n");
		printf("\t0. Admin\n\t1. Agent\n\t2. Customer\n");
		printf("\tEnter your choice here : ");
		scanf("%d", &user_type);
		printf("\tEnter your user name here : ");
		scanf("%s", user_name);
		strcpy(user_pass,getpass("\tEnter your password here : "));
		write(ser_socket, &user_type, sizeof(user_type));
		write(ser_socket, &user_name, sizeof(user_name));
		write(ser_socket, &user_pass, strlen(user_pass));
		
		read(ser_socket, &user_id, sizeof(user_id));
		printf("\t Your login id to  sign in is :: %d\n", user_id);
		return 2;
	}
	else							
		return 3;
	
}

// Main menu //

int menu(int ser_socket,int user_type){
	int ch;
	if(user_type==2 || user_type==1){					// Agent and Customer
		printf("\t1. Book Ticket\n");
		printf("\t2. View your  Bookings\n");
		printf("\t3. Update your Booking\n");
		printf("\t4. Cancel your ticket \n");
		printf("\t5. Logout\n");
		printf("\tEnter your choice here : ");
		scanf("%d",&ch);
		write(ser_socket,&ch,sizeof(ch));
		return book_tkt(ser_socket,ch);
	}
	else if(user_type==0){					// Admin
		printf("\n\t1.Operations to be performed on Trains \n");
		printf("\t2.Operations to be performed on User\n");
		printf("\t3.Logout\n");
		printf("\t Enter your choice here: ");
		scanf("%d",&ch);
		write(ser_socket,&ch,sizeof(ch));
			if(ch==1){
				printf("\t1. Add a new train\n");
				printf("\t2. Search for a train\n");
				printf("\t3. View list of trains\n");
				printf("\t4. Modify a train name or train seats\n");
				printf("\t5. Delete a train\n");
				
				printf("\t Enter your choice here : ");
				scanf("%d",&ch);	
				write(ser_socket,&ch,sizeof(ch));
				return trainOperations(ser_socket,ch);
			}
			else if(ch==2){
				printf("\t1. Add a new User\n");
				printf("\t2. View all users\n");
				printf("\t3. Modify an user\n");
				printf("\t4. Delete an user\n");
				printf("\t5. Search for an user\n");
				
				printf("\t Enter your choice here : ");
				scanf("%d",&ch);
				write(ser_socket,&ch,sizeof(ch));
				return userOperations(ser_socket,ch);
			
			}
			else if(ch==3)
				return -1;
	}	
	
}

// operations on user//
int userOperations(int ser_socket,int ch){
	int flag1 = 0;
	if(ch==1){							// Add user
		int type,id;
		char name[50],password[50];
		printf("\n\tEnter The Type Of Account:: \n");
		printf("\t1. Agent\n\t2. Customer\n");
		printf("\tYour Response: ");
		scanf("%d", &type);
		printf("\tUser Name: ");
		scanf("%s", name);
		strcpy(password,getpass("\tPassword: "));
		write(ser_socket, &type, sizeof(type));
		write(ser_socket, &name, sizeof(name));
		write(ser_socket, &password, strlen(password));
		read(ser_socket,&flag1,sizeof(flag1));	
		if(flag1){
			read(ser_socket,&id,sizeof(id));
			printf("\tRemember Your login id For Further Logins as :: %d\n", id);
		}
		return flag1;	
	}
	
	
	else if(ch==2){						//  user list
		int no_of_users;
		int id,type;
		char user_name[50];
		read(ser_socket,&no_of_users,sizeof(no_of_users));

		printf("\tU_id\tU_name\tU_type\n");
		while(no_of_users--){
			read(ser_socket,&id,sizeof(id));
			read(ser_socket,&user_name,sizeof(user_name));
			read(ser_socket,&type,sizeof(type));
			
			if(strcmp(user_name, "deleted")!=0)
				printf("\t%d\t%s\t%d\n",id,user_name,type);
		}

		return flag1;	
	}

	else if (ch==3){						// Update  an user
		int ch=2,flag1=0,user_id;
		char name[50],pass[50];
		write(ser_socket,&ch,sizeof(int));
		userOperations(ser_socket,ch);
		printf("\n\t Enter the user_id you want to modify: ");
		scanf("%d",&user_id);
		write(ser_socket,&user_id,sizeof(user_id));
		
		printf("\n\t1. User Name\n\t2. Password\n");
		printf("\t Your Choice: ");
		scanf("%d",&ch);
		write(ser_socket,&ch,sizeof(ch));
		
		if(ch==1){
			read(ser_socket,&name,sizeof(name));
			printf("\n\t Current name: %s",name);
			printf("\n\t Updated name:");
			scanf("%s",name);
			write(ser_socket,&name,sizeof(name));
			read(ser_socket,&flag1,sizeof(flag1));
		}
		
		else if(ch==2){
			printf("\n\t Enter Current password: ");
			scanf("%s",pass);
			write(ser_socket,&pass,sizeof(pass));
			read(ser_socket,&flag1,sizeof(flag1));
			if(flag1){
				printf("\n\t Enter new password:");
				scanf("%s",pass);
			}
			else
				printf("\n\tIncorrect password\n");
			
			write(ser_socket,&pass,sizeof(pass));
		}
		if(flag1){
			read(ser_socket,&flag1,sizeof(flag1));
			if(flag1)
				printf("\n\t User data updated successfully\n");
		}
		return flag1;
	}

	else if(ch==4){						// Delete an user
		int ch=2,user_id,flag1=0;
		write(ser_socket,&ch,sizeof(int));
		userOperations(ser_socket,ch);
		
		printf("\n\t Enter the user id you want to delete: ");
		scanf("%d",&user_id);
		write(ser_socket,&user_id,sizeof(user_id));
		read(ser_socket,&flag1,sizeof(flag1));
		if(flag1)
			printf("\n\t User deleted successfully\n");
		return flag1;
	}
	else if(ch==5){						// Search for an  user 
		int no_of_users;
		int id,type;
		int flag=0;
		char user_name[50];
		char search_user[50];
		read(ser_socket,&no_of_users,sizeof(no_of_users));
		printf("\n\t Enter user name to search : ");
		scanf("%s",search_user);

		while(no_of_users--){
			read(ser_socket,&id,sizeof(id));
			read(ser_socket,&user_name,sizeof(user_name));
			read(ser_socket,&type,sizeof(type));
			
			if(strcmp(user_name, search_user)==0)
			{
				printf("\tU_id\tU_name\tU_type\n");
				printf("\t%d\t%s\t%d\n",id,user_name,type);
				flag=1;
				
				}
		}
		if (flag==0) {
			printf("\n\t User %s is not found",search_user);
		             }

		return flag1;	
	}

}

//operations on train //
int trainOperations(int ser_socket,int ch){
	int flag1 = 0;
	if(ch==1){				// Add train 
		char train_name[50];
		printf("\n\tEnter train name: ");
		scanf("%s",train_name);
		write(ser_socket, &train_name, sizeof(train_name));
		read(ser_socket,&flag1,sizeof(flag1));	
		if(flag1)
			printf("\n\tTrain added successfully\n");

		return flag1;	
	}
	
	else if(ch==2){			// search train 
		int num_trains;
		int train_num;
		int flag=0;
		char train_name[50];
		char search_train[50];
		int total_seats;
		int av_seats;
		read(ser_socket,&num_trains,sizeof(num_trains));
		printf("\n\t Enter train name to search : ");
		scanf("%s",search_train);

		printf("\tT_no\tT_name\tT_seats\tA_seats\n");
		while(num_trains--){
			read(ser_socket,&train_num,sizeof(train_num));
			read(ser_socket,&train_name,sizeof(train_name));
			read(ser_socket,&total_seats,sizeof(total_seats));
			read(ser_socket,&av_seats,sizeof(av_seats));
			
			if(strcmp(train_name, search_train)==0)
			       {
			       
			       printf("\t%d\t%s\t%d\t%d\n",train_num,train_name,total_seats,av_seats);
			       flag=1;
			       }
			}
			if (flag==0) {
			printf("\n\t Train %s is not found",search_train);
		             }

		return flag1;	
	}
	else if(ch==3){			// View train 
		int num_trains;
		int train_num;
		char train_name[50];
		int total_seats;
		int flag=0;
		int av_seats;
		read(ser_socket,&num_trains,sizeof(num_trains));

		printf("\tT_no\tT_name\tT_seats\tA_seats\n");
		while(num_trains--){
			read(ser_socket,&train_num,sizeof(train_num));
			read(ser_socket,&train_name,sizeof(train_name));
			read(ser_socket,&total_seats,sizeof(total_seats));
			read(ser_socket,&av_seats,sizeof(av_seats));
			
			if(strcmp(train_name, "deleted")!=0)
				printf("\t%d\t%s\t%d\t%d\n",train_num,train_name,total_seats,av_seats);
				 flag=1;
		}

               if (flag==0) {
			printf("\n\t Train list is empty");
		             }

		return flag1;	
	}
	
	else if (ch==4){			// Update train 
		int total_seats,ch=3,flag1=0,train_id;
		char train_name[50];
		int val;
		write(ser_socket,&ch,sizeof(int));
		trainOperations(ser_socket,ch);
		printf("\n\t Enter the train number you want to modify: ");
		scanf("%d",&train_id);
		write(ser_socket,&train_id,sizeof(train_id));
		
		printf("\n\t1. Train Name\n\t2. Total Seats\n");
		printf("\t Your Choice: ");
		scanf("%d",&ch);
		write(ser_socket,&ch,sizeof(ch));
		
		if(ch==1){
			read(ser_socket,train_name,sizeof(train_name));
			printf("\n\t Current name: %s",train_name);
			printf("\n\t Enter the name to be updated:");
			scanf("%s",train_name);
			write(ser_socket,train_name,sizeof(train_name));
		}
		else if(ch==2){
		        printf("\n\t Enter no. of total seats to increase : ");
			scanf("%d",&val);
			write(ser_socket,&val,sizeof(val));
		
		}
		read(ser_socket,&flag1,sizeof(flag1));
		if(flag1)
			printf("\n\t Train data updated successfully\n");
		return flag1;
	}

	else if(ch==5){				// Delete train response
		int ch=2,train_id,flag1=0;
		write(ser_socket,&ch,sizeof(int));
		trainOperations(ser_socket,ch);
		
		printf("\n\t Enter the train number you want to delete: ");
		scanf("%d",&train_id);
		write(ser_socket,&train_id,sizeof(train_id));
		read(ser_socket,&flag1,sizeof(flag1));
		if(flag1)
			printf("\n\t Train deleted successfully\n");
		return flag1;
	}
	
		
	
}



//Booking tickets//
int book_tkt(int ser_socket,int ch){
	int flag1 =0;
	if(ch==1){										// Book tickets
		int view=3,train_id,seats;
		write(ser_socket,&view,sizeof(int));
		trainOperations(ser_socket,view);
		printf("\n\tEnter the train number you want to book: ");
		scanf("%d",&train_id);
		write(ser_socket,&train_id,sizeof(train_id));
				
		printf("\n\tEnter the no. of seats you want to book: ");
		scanf("%d",&seats);
		write(ser_socket,&seats,sizeof(seats));
	
		read(ser_socket,&flag1,sizeof(flag1));
		if(flag1)
			printf("\n\tTickets booked successfully.\n");
		else
			printf("\n\total_seats were not available.\n");

		return flag1;
	}
	
	else if(ch==2){									// View bookings
		int no_of_bookings;
		int id,train_id,seats;
		int flag=0;
		read(ser_socket,&no_of_bookings,sizeof(no_of_bookings));

		printf("\tB_id\tT_no\total_seats\n");
		while(no_of_bookings--){
			read(ser_socket,&id,sizeof(id));
			read(ser_socket,&train_id,sizeof(train_id));
			read(ser_socket,&seats,sizeof(seats));
			
			if(seats!=0)
			{
			        
				printf("\t%d\t%d\t%d\n",id,train_id,seats);
				flag=1;
				}
		}
		if(flag==0)
		{
		printf("\n\t no bookings found \n");
		}

		return flag1;
	}

	else if(ch==3){									// Update a booking 
		int ch = 2,booking_id,val,flag1;
		book_tkt(ser_socket,ch);
		printf("\n\t Enter the booking id you wish to modify: ");
		scanf("%d",&booking_id);
		write(ser_socket,&booking_id,sizeof(booking_id));

		printf("\n\t1. Increase number of seats\n\t2. Decrease number of seats\n");
		printf("\t Your Choice: ");
		scanf("%d",&ch);
		write(ser_socket,&ch,sizeof(ch));

		if(ch==1){
			printf("\n\t Enter no.of tickets to increase : ");
			scanf("%d",&val);
			write(ser_socket,&val,sizeof(val));
		}
		else if(ch==2){
			printf("\n\t Enter no.of tickets to decrease : ");
			scanf("%d",&val);
			write(ser_socket,&val,sizeof(val));
		}
		read(ser_socket,&flag1,sizeof(flag1));
		if(flag1)
			printf("\n\tBooking updated successfully.\n");
		else
			printf("\n\t Updation failed. No more seats available.\n");
		return flag1;
	}
	
	else if(ch==4){									// Cancel  booking
		int ch = 2,booking_id,flag1;
		book_tkt(ser_socket,ch);
		printf("\n\t Enter the booking id you want to cancel: ");
		scanf("%d",&booking_id);
		write(ser_socket,&booking_id,sizeof(booking_id));
		read(ser_socket,&flag1,sizeof(flag1));
		if(flag1)
			printf("\n\tBooking cancelled successfully.\n");
		else
			printf("\n\tCancellation failed.\n");
		return flag1;
	}
	else if(ch==5)									
		return -1;
	
}
int main(void) { 
	int ser_socket; 
    	struct sockaddr_in server; 
    	char server_reply[50];
	
    	ser_socket = socket(AF_INET, SOCK_STREAM, 0); 
    	perror("ser_socket : ");
    
    	server.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    	server.sin_family = AF_INET; 
    	server.sin_port = htons(8000); 
   
    	connect(ser_socket, (struct sockaddr*)&server, sizeof(server));
       	perror("connect : "); 
    
	while(client(ser_socket)!=3);
    	close(ser_socket); 
    	
	return 0; 
} 

