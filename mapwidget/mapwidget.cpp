#include <math.h>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include "mapwidget.h"
#include "mapdefinition.h"
#include "ui_mapwidget.h"

MapWidget::MapWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::MapWidget), mapScene(new QGraphicsScene(this))
{
    //indicar la interfaz de usuario del widget
    ui->setupUi(this);

    //cargar la imagen en el atributo QImage (imagen ya añadida como recurso)
    mapImage.load(MAP_IMAGE_PATH);

    //convertir a tipo pixmap para poder añadirla a la escena gráfica
    QPixmap mapPixmap = QPixmap::fromImage(mapImage);
    mapScene.addPixmap(mapPixmap);

    //ajustar el rectángulo contenedor de la escena al tamaño exacto de la imagen
    mapScene.setSceneRect(mapPixmap.rect());

    //decirle al graphicsview de la interfaz de usuario que su escena es esta
    ui->mapView->setScene(&mapScene);

    //pintado ejemplo circulo
    //paintCircles();

    //prueba de precision de coordenadas-pixel
    //testPixelPrecision();

    //prueba de colocación origen en el mapa.
    testOrigen();
}

MapWidget::~MapWidget()
{
    delete ui;
}

/*
 * Funcion privada para probar la precision de la conversion de coordenadas a pixeles de la imagen
 */
void MapWidget::testPixelPrecision(){
    //prueba coordenadas salida por consola
    //esquinas
    std::cout << convertToDecimalDegrees(38,0,0)<< std::endl;
    std::cout << convertToDecimalDegrees(-14,0,0)<< std::endl;
    std::cout << convertToDecimalDegrees(34,0,0)<< std::endl;
    std::cout << convertToDecimalDegrees(-3,0,0)<< std::endl;

    //cabo san vicente 36°59'39.61" (36.994336) y -8°-56'-9.60" (-8.936) -> por google maps
    std::cout << convertToDecimalDegrees(36,59,39.61)<< std::endl;
    std::cout << convertToDecimalDegrees(-8,-56,-9.6)<< std::endl;
    long double x,y;
    coordinatesToPixels(x,y,36.994336,-8.936);
    std::cout << x << ' ' << y << std::endl;
    //pintado ejemplo coordenadas (linea desde cabo san vicente hasta esquina izquierda arriba)
    mapScene.addLine (x,y,0,0);

    //punta gibraltar 36° 6'34.01" (36.109447) -5°-20'-43.59" (-5.345442) -> por google maps
    std::cout << convertToDecimalDegrees(36,6,34.01)<< std::endl;
    std::cout << convertToDecimalDegrees(-5,-20,-43.59)<< std::endl;
    long double x2,y2;
    coordinatesToPixels(x2,y2,36.109447,-5.345442);
    std::cout << x2 << ' ' << y2 << std::endl;
    //pintado ejemplo coordenadas (linea desde gibraltar hasta derecha abajo
    mapScene.addLine (x2,y2,mapScene.width (),mapScene.height ());

    //Punta san felipe Cádiz 36°32'16.12"  -6°-18'-1.20" -> por google maps
    std::cout << convertToDecimalDegrees(36,32,16.12)<< std::endl;
    std::cout << convertToDecimalDegrees(-6,-18,-1.20)<< std::endl;
    long double x3,y3;
    coordinatesToPixels(x3,y3,36,32,16.12,-6,-18,-1.20);
    std::cout << x3 << ' ' << y3 << std::endl;
    //pintado ejemplo coordenadas (linea desde cadiz hasta izquierda abajo
    mapScene.addLine (x3,y3,0,mapScene.height ());
}

/*
 * Funcion privada para probar la colocacion de un Origen en la imagen
 */
void MapWidget::testOrigen(){
    std::set<Station> mystations;
    mystations.insert(Station("0x0000", "0x0001", 36.00204023875479, -10.2456402219765, 0));
    mystations.insert(Station("0x0001", "0x0001", 37.00204023875479, -12.2456402219765, 1));
    mystations.insert(Station("0x0002", "0x0002", 33.00204023875479, -5.2456402219765, 2));
    mystations.insert(Station("0x0003", "0x0003", 30.00204023875479, -8.2456402219765, 3));

    // getting time from system (best to test it):
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);

    //System time - 3.
    timeinfo->tm_sec -= 3;

    // getting time from string:
    //struct std::tm tm;
    //std::istringstream ss("2014-12-10 13:54:00");
    //ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

    //Punta san felipe Cádiz 36°32'16.12"  -6°-18'-1.20" -> por google maps
    Origin myOrigin("0x0001b",*timeinfo, convertToDecimalDegrees(36,32,16.12), convertToDecimalDegrees(-6,-18,-1.20), 3.54, mystations);
    paintOrigin(myOrigin);
}


/**COORDENADAS->PIXELES**/

/**funcion que convierte grados minutos y segundos a grados en decima*/
long double MapWidget::convertToDecimalDegrees(long double degrees, long double minutes, long double seconds){
    return degrees + (minutes/60) + (seconds/3600);
}

/*funcion que interpola y transforma las cooordenadas geográficas que se le indiquen a pixeles de la imagen
 *--version recibiendo las coordenadas en grados minutos y segundos
*/
void MapWidget::coordinatesToPixels(long double &pixelX,long double &pixelY,
                                    long double degreesTargetLat,long double minutesTargetLat,long double secondsTargetLat,
                                    long double degreesTargetLon, long double minutesTargetLon,long double secondsTargetLon)
{
    long double minLon = MAP_MIN_LONGITUDE;
    long double minLat = MAP_MIN_LATITUDE;
    long double maxLon = MAP_MAX_LONGITUDE;
    long double maxLat = MAP_MAX_LATITUDE;
    long double targetLon = convertToDecimalDegrees(degreesTargetLon, minutesTargetLon, secondsTargetLon);
    long double targetLat = convertToDecimalDegrees(degreesTargetLat, minutesTargetLat, secondsTargetLat);
    long double maxXpixel = mapScene.width();
    long double maxYpixel = mapScene.height();
    long double minXpixel = 0;
    long double minYpixel = 0;

    pixelX = ((targetLon - minLon) / (maxLon - minLon)) * (maxXpixel - minXpixel);
    pixelY = ((targetLat - minLat) / (maxLat - minLat)) * (maxYpixel - minYpixel);
}

/*funcion que interpola y transforma las cooordenadas geográficas que se le indiquen a pixeles de la imagen
 *--version recibiendo las coordenadas ya en decimal
*/
void MapWidget::coordinatesToPixels(long double &pixelX,long double &pixelY,
                                    long double targetLat,long double targetLon)
{
    long double minLon = MAP_MIN_LONGITUDE;
    long double minLat = MAP_MIN_LATITUDE;
    long double maxLon = MAP_MAX_LONGITUDE;
    long double maxLat = MAP_MAX_LATITUDE;
    long double maxXpixel = mapScene.width();
    long double maxYpixel = mapScene.height();
    long double minXpixel = 0;
    long double minYpixel = 0;

    pixelX = ((targetLon - minLon) / (maxLon - minLon)) * (maxXpixel - minXpixel);
    pixelY = ((targetLat - minLat) / (maxLat - minLat)) * (maxYpixel - minYpixel);
}





/**ESTACIONES**/
void MapWidget::paintStations(const std::set<Station> &stations){
    this->stations = stations;
    //for(std::set<Station>::iterator it=origin.stations.begin(); it!=origin.stations.end(); ++it)
        //os << *it;

    //completar método para pintar las estaciones
    //hay que pintar triangulo en posicion (latitud y longitud a pixeles)
    //hay que pintarlas con el color negro inicialmente
}

/**ORIGEN**/
void MapWidget::paintOrigin(const Origin &origin){
    long double coordX, coordY;

    this->currentOrigin = origin;
    coordinatesToPixels(coordX,coordY,currentOrigin.getLatitude(),currentOrigin.getLongitude());
    //pintamos las estaciones con su nuevo color
    paintStations(currentOrigin.getStations());
    // pintamos el primer circulo con gris (cambiable en paintCircles) opacidad 200/256.
    paintCircles(coordX, coordY, calculateRadius(),100, 0, 0, 128);
    //pintamos el epicentro (latitud y longitud a pixeles)
    paintCircles(coordX, coordY, 2, 255, 128, 0, 0);
}



/**CIRCULOS CONCENTRICOS**/

/**funcion que calcula el radio del círculo de expansion
 * fórmula-> radio = (tiempo actual - tiempo origen) * velocidad de propagacion de la onda
 * la diferencia de tiempo habrá que medirla dependiendo de las unidades de la velocidad
 */
float MapWidget::calculateRadius(){
    double speed = 3000; //velocidad de propagacion Onda S: 3000m/s aproximadamente.
    long double radius;
    long int difSeconds=0;
    time_t rawtime;
    struct tm * timeinfo;

    // Getting the system time and the origin time diference (only h/m/s).
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    std::cout << timeinfo->tm_hour << " -- " <<  timeinfo->tm_min << " -- " <<  timeinfo->tm_sec << " -- " << std::endl;
    difSeconds += timeinfo->tm_sec - currentOrigin.getOriginTime().tm_sec;
    difSeconds += (timeinfo->tm_min - currentOrigin.getOriginTime().tm_min)*60;
    difSeconds += (timeinfo->tm_hour - currentOrigin.getOriginTime().tm_hour)*3600;

    // getting the radius in meters.
    radius = difSeconds * speed;

    // Calculate the numbers of pixels to "Radius meters".
    return (radius*mapScene.width())/MAP_METRES_LONGITUDE;

}

/**funcion que pinta un circulo de expansion
 * centro en epicentro
 * radio calculado con la funcion calculateRadius()
 * FALTA HACERLA AUTOMATICA CADA 5s
 */
void MapWidget::paintCircles(){

    //definir el centro del círculo: coordenadas del origen/evento
    //pasar las coordenadas almacenadas en el atributo currentOrigin a pixeles
    //y despues pasarselas al constructor de center
    /*
    long double x,y;
    coordinatesToPixels(x,y,currentOrigin.getLatitude(),currentOrigin.getLongitude());
    QPoint center(x,y);
    */
    //--temporalmente definido el centro en el centro de la imagen
    QPoint center(mapScene.width()/2, mapScene.height ()/2);

    //definir radio del círculo
    float radius = 10;

    //rectágulo que va a contener la elipse,
    //coordenada superior 0,0 y tamaño 2*radio tanto alto como ancho
    QRect rect(0,0,2*radius,2*radius);

    //ESTO ES CLAVE: mover el rectágulo contenedor de manera que el centro del circulo sea el deseado
    rect.moveCenter(center);

    //pintar el circulo sobre la escena que contiene el mapa
    mapScene.addEllipse (rect);
}


void MapWidget::paintCircles(const long double& x, const long double& y, const long double& radius, int transparence, int red, int green, int blue){

    //definir el centro del círculo: coordenadas del origen/evento
    //pasar las coordenadas almacenadas en el atributo currentOrigin a pixeles
    //y despues pasarselas al constructor de center
    /*
    long double x,y;
    coordinatesToPixels(x,y,currentOrigin.getLatitude(),currentOrigin.getLongitude());
    QPoint center(x,y);
    */
    //--temporalmente definido el centro en el centro de la imagen
    QPoint center(x, y);

    //rectágulo que va a contener la elipse,
    //coordenada superior 0,0 y tamaño 2*radio tanto alto como ancho
    QRect rect(0,0,2*radius,2*radius);


    //ESTO ES CLAVE: mover el rectágulo contenedor de manera que el centro del circulo sea el deseado
    rect.moveCenter(center);

    //pintar el circulo sobre la escena que contiene el mapa
    mapScene.addEllipse (rect,QPen(),QBrush(QColor(red,green,blue,transparence)));
}
