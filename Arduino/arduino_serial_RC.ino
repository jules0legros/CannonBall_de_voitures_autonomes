#include <Servo.h>

#define  baudrate 9600  // com port speed a changer en ad�quation avec le cpp

Servo servoTest; //Servo pour les test
Servo direction; //Servo de la direction
Servo propulsion; //Servo controlant la propulsion

int j = 0;
bool detected,stopSignal = false;
int vit;
int vit_max = 75;
unsigned long size = 0;  // taille du marker color� a tracker
unsigned long pos = 0;   // position du marker color� en pixel (sur 640)
int ch1; 	// Nos deux cannaux de RC
int ch2;	// Nos deux cannaux de RC
//----------------------------------------------------------------
// utiles pour lire les donn�es du port serie
short MSB = 0;  // pour construire un entier de 2 octets depuis le port serie en octet
short LSB = 0;  // pour construire un entier de 2 octets depuis le port serie en octet
int   MSBLSB = 0;  // pour construire un entier de 2 octets depuis le port serie en octet
//----------------------------------------------------------------



void setup(){
	// Brancher le servo de test � la pin 7
	// Attacher le servo de direction � la pin 8 
	// Attacher le servo controlant la propulsion � la pin 9
	// Attention � l'initialisation des angles qui est fix�e � 90 degr�s par d�faut. 
	pinMode(7, OUTPUT);
	servoTest.attach(7);
	servoTest.write(90);
	pinMode(8, OUTPUT);
	direction.attach(8); 
	direction.write(90);
	pinMode(9, OUTPUT);
	propulsion.attach(9); // attention  particuli�rement � cette initialisation, garder les roues motrices en l'air par pr�caution
	propulsion.write(90);
	
	  //Relier les masses du r�cepteur et de l'arduino, et
	pinMode(5, INPUT);  // ch1 sur Arduino pin5 direction
	pinMode(6, INPUT);  // ch2 sur Arduino pin6 propuslion
	
	Serial.begin(baudrate);        // connection au serial port
	Serial.println("Starting Cannonball");

}

bool tooFast(){
	vit = propulsion.read();
	if (vit < vit_max){return true; }
	else{return false; }
}

//permet d'arreter le vehicule
void stop(){
	propulsion.write(90);
}

// arrete completement la simulation
void finalStop(){
	int s;
	int	d;
	int	p;
	propulsion.write(90);
	direction.write(90);
	s = servoTest.attached();
	servoTest.detach();
	d = direction.attached();
	direction.detach();
	p = propulsion.attached();
	propulsion.detach();
}
// permet d'accc�l�rer
void accelerate(){
	int oldAngle;
	oldAngle = propulsion.read();
	propulsion.write(oldAngle-8);
}

// permet d'acc�l�rer de fa�on plus douce (avec le temps d'attente en ms)
void accelerateSmooth(unsigned long time){
	int oldAngle;
	oldAngle = propulsion.read();
	propulsion.write(oldAngle - 8);
	delay(time);
}

// permet de dec�l�rer
void decelerate(){
	int oldAngle;
	oldAngle = propulsion.read();
	propulsion.write(oldAngle+8);
}

// permet de dec�l�rer de fa�on plus douce (avec le temps d'attente en ms)
void decelerateSmooth(unsigned long time){
	int oldAngle;
	oldAngle = propulsion.read();
	propulsion.write(oldAngle + 8);
	delay(time);
}

// permet de tourner les roues � droite suivant un angle (0-90)
void turnright(int angle){
	direction.write(90+angle);
}

// permet de tourner les roues � gauche suivant un angle (0-90)
void turnleft(int angle){
	direction.write(90-angle);
}


// la routine pour la recherche de markers
void routine(){
	int t = 5000; // temps pour faire une boucle suivant la vitesse
	int v = 82; // g�re la vitesse de la voiture
	propulsion.write(v);
	//propulsion.writeMicroseconds(1456); 
	direction.write(45);
	delay(t);
	direction.write(135);
	delay(t);
}

void RCUpdate(){
  //Pulsin n'est pas tr�s optimis� mais �a fonctionne
  //Signaux de largeur modul�e (1ms � 2ms) sur cycle 20ms
  //Donc 1000 microsec = z�ro, 2000 micsec = max et 1500 = neutre
  ch1 = pulseIn(5, HIGH, 25000); // Lire ch1 (25 ms)
  ch2 = pulseIn(6, HIGH, 25000); // Lire ch2
}

void emergency(){
	RCUpdate()
	if(ch1>1750 && ch1<1250 && ch2<1250 && ch2>1750){
	stopSignal = true;
	}
}


void loop() {
	emergency();
	if (!stopSignal){
	while (Serial.available() <= 0); // en attente de donn�es depuis le port s�rie
	if (Serial.available() > 4)  // en attente de 4 octets (2 pour la taille 2 pour la position relative)
	{
		emergency();
		if (!stopSignal){
		// obtention du int de 2 octets de la position relative depuis le port serie
		MSB = Serial.read();
//		delay(5);
		LSB = Serial.read();
		MSBLSB = word(MSB, LSB);
		pos = MSBLSB;
//		delay(5);
		}
		
		emergency();
		if (!stopSignal){
		if (pos<641 && pos>-1) { // pour eviter les erreurs de positionnement
		int posR = (int) pos * 90 / 640 + 45 ; 	//on a une position en fonction de la taille de l'�cran (640)
        direction.write(posR); 					// l'angle de braquage va de 45 a 135 (range=90)
		}
		}
		
		emergency();
		if (!stopSignal){
		// obtention du int de 2 octets de la taille depuis le port serie
		MSB = Serial.read();
//		delay(5);
		LSB = Serial.read();
		MSBLSB = word(MSB, LSB);
		size = MSBLSB;
//		delay(5);
		}
		
		emergency();
		if (!stopSignal){
		if (size<11500 && size>1500) {  //pour ne pas que la voiture aille trop vite
			
			// on a une vitesse en fonction de la taille de l'objet (plus il est gros, donc proche, moins elle est elevee)
			// on fixe un angle de vit_max comme vitesse max et 1500 comme valeure min de taille d'objet pour avancer a la vitesse max
			// avec une taille de 11500, la vitesse est nulle (objet trop pres) l'angle est de 90 (on entre dans la boucle suivante)
			
			emergency();
			if (!stopSignal){
			int puiss = (int) (size-1500)/500 + vit_max;	
			propulsion.write(puiss);				
			}
		}else if (size >=11500 && size <=1500) { // objet tres pres ou objet perdu (ou trop loin...)
			
			emergency();
			if (!stopSignal){
			propulsion.write(90);
			}
		}
		}
/*
		if (!tooFast()){
			
		}
		else if (tooFast()){ //la vitesse est un angle negatif
			while (tooFast())
				decelerate(); // reduire vitesse du moteur de propulsion
		}*/
	}
	}
}
