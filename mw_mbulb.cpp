/*
Copyright 2019, 2020 Sebastian Motzet

This file is part of MandelbulbUI.

MandelbulbUI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

MandelbulbUI is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MandelbulbUI.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

//Saving/Loading:
void MainWindow::actionSaveMBulb(){
    QFileDialog saveDialog;
    saveDialog.setDefaultSuffix("txt");
    QString fileName = saveDialog.getSaveFileName();
    std::string filePath = fileName.toStdString();
    std::string extension = getFileExt(filePath, "txt");
    if(extension == "txt"){
        mBulb.saveCoords(filePath);
    }
    else if(extension == "bin"){
        mBulb.saveInternal(filePath);
    }
    ui->label_infoText->setText(QString::fromStdString("Saved mandelbulb to " + filePath ));
}
void MainWindow::actionLoadMBulb(){
    QFileDialog loadDialog;
    loadDialog.setDefaultSuffix("bin");
    QString fileName = loadDialog.getOpenFileName();
    std::string filePath = fileName.toStdString();
    std::string extension = getFileExt(filePath);
    if(extension == "bin"){
        mBulb.loadInternal(filePath);
        MBulbToGraph();
        ui->label_infoText->setText(QString::fromStdString("Loaded " + filePath ));
    }
    else{
        ui->label_infoText->setText(QString::fromStdString("ERROR: Invalid file extension: " + extension));
    }

}
//Show in scatter graph:
void MainWindow::MBulbToGraph(){
    scatterSeries.dataProxy()->removeItems(0,scatterSeries.dataProxy()->itemCount());
    QtDataVisualization::QScatterDataArray scatterData;
    for(int i = 0; i < mBulb.xsize; ++i){
    for(int j = 0; j < mBulb.ysize; ++j){
    for(int k = 0; k < mBulb.zsize; ++k){
        ivec index = {i,j,k};
        if(mBulb.getState(index)){
            dvec coords(3);
            mBulb.convIndexToCoord(index,coords);
            scatterData << QVector3D(coords[0], coords[1], coords[2]);
        }
    }
    }
    }
    scatterSeries.dataProxy()->addItems(scatterData);
    scatterGraph.addSeries(&scatterSeries);
    ui->pushButton_filterHull->setEnabled(true);
}
//Delete cache:
void MainWindow::delMBulbCache(){
    mBulb.remove();
    ui->actionSave_Mandelbulb->setEnabled(false);
    ui->pushButton_filterHull->setEnabled(false);
    ui->pushButton_generate->setEnabled(true);
    ui->label_infoText->setText("Deleted mandelbulb cache.");
}
//Algorithm:
void MainWindow::calcMBulbUI(){
    //See if 'auto' is checked:
    bool autoHull = ui->checkBox_autoHull->isChecked();
    //Adapt UI:
    ui->pushButton_generate->setEnabled(false);
    if(autoHull){
        ui->pushButton_filterHull->setEnabled(false);
        ui->checkBox_autoHull->setEnabled(false);
    }
    //Get input values:
    int res = ui->spinBox_res->value();
    int iter = ui->spinBox_iter->value();
    double min = ui->doubleSpinBox_min->value();
    double max = ui->doubleSpinBox_max->value();
    double power = ui->doubleSpinBox_power->value();
    double maxLength = ui->doubleSpinBox_maxLength->value();
    double distance = std::abs(max - min) / res;
    dvec vDist(3,distance);
    dvec vMin(3,min);
    ivec vSize(3,res);
    //Save input values:
    inputValues.res = res;
    inputValues.iter = iter;
    inputValues.min = min;
    inputValues.max = max;
    inputValues.power = power;
    inputValues.maxLength = maxLength;
    //Reset primCloud:
    mBulb.remove();
    mBulb.init(vDist,vMin,vSize);
    ui->label_setPoints->setText("0");
    ui->label_infoText->setText("Generating Mandelbulb...");
    //Initialize Scatter series:
    scatterSeries.dataProxy()->removeItems(0,scatterSeries.dataProxy()->itemCount());
    QtDataVisualization::QScatterDataArray scatterData;

    //Calculation:
          double density = 0;
          double xpos, ypos, zpos;
          double xpoint, ypoint, zpoint;
          double cx, cy, cz;
          double r, phi, theta;

          //Track progress:
          double progress = 0.0;
          double progdiv = 100.0 / pow(double(res - 3), 3.0);
          int pointCount = 0;

          for(xpos = min; xpos <= max; xpos += distance){
            for(ypos = min; ypos <= max; ypos += distance){
            for(zpos = min; zpos <= max; zpos += distance){
                //reset for next point:
                xpoint = xpos;
                ypoint = ypos;
                zpoint = zpos;
                cx = xpos;
                cy = ypos;
                cz = zpos;

                //Sequence loop:
                for(int i = 0; i <= iter; i++)
                {
                    r = sqrt(xpoint*xpoint + ypoint*ypoint + zpoint*zpoint);
                    phi = atan(ypoint/xpoint);
                    theta = acos(zpoint/r);

                    xpoint = pow(r, power) * sin(power * theta) * cos(power * phi) + cx;
                    ypoint = pow(r, power) * sin(power * theta) * sin(power * phi) + cy;
                    zpoint = pow(r, power) * cos(power * theta) + cz;

                    if(r >= maxLength)
                    {
                        density = 0.0;
                        break;
                    }
                    density = 1.0;

                }

                //Display progress:
                progress += progdiv;
                ui->progressBar->setValue(progress);

                if(density == 1.0)
                {
                    //Update Set:
                    pointCount++;
                    ui->label_setPoints->setText(QString::number(pointCount));
                    dvec coord = {xpos,ypos,zpos};
                    mBulb.setState(coord,true);
                    scatterData << QVector3D(xpos,ypos,zpos);
                }

            }//zpos loop end
            }//ypos loop end
            }//xpos loop end
          scatterSeries.dataProxy()->addItems(scatterData);
          scatterGraph.addSeries(&scatterSeries);
          ui->label_infoText->setText("Generated Mandelbulb");
          ui->actionSave_Mandelbulb->setEnabled(true);
          if(autoHull){
              calcHullUI();
          }else{
              ui->pushButton_filterHull->setEnabled(true);
          }
}