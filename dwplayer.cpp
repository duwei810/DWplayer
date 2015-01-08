#include "dwplayer.h"
#include "ui_dwplayer.h"

DWplayer::DWplayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DWplayer)
{
    ui->setupUi(this);
    isPlaying=false;
    playingfile=QString();
    isSwitch=false;
    playingId=0;
    isRepeatedALL=true;
    isRepeatedOne=false;
    connect(&playerProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(getPlayerFinished(int,QProcess::ExitStatus)));
    connect(ui->musiclist,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(getDoubleclicked(QListWidgetItem*)));
    playerProcess.setProcessChannelMode(QProcess::MergedChannels);
    connect(&playerProcess,SIGNAL(readyReadStandardOutput()),this,SLOT(readFromStout()));
    connect(ui->pPlayerSlider,SIGNAL(sliderMoved(int)),this,SLOT(updateSlider(int)));
    connect(&timer,SIGNAL(timeout()),this,SLOT(updateSlider()));
    connect(ui->voiceSlider,SIGNAL(sliderMoved(int)),this,SLOT(updateVol(int)));
}

DWplayer::~DWplayer()
{
    delete ui;
}

void DWplayer::on_pBtnOpen_clicked()
{
    playingfile=QFileDialog::getOpenFileName(this,"open","/music","AV files(*.mp3);;ALL(*)");
    isSwitch=true; //处于歌曲切换状态
    if(playingfile.isEmpty())
        return;
    for(int i=0;i<ui->musiclist->count();i++)
    {
        if(playingfile==ui->musiclist->item(i)->whatsThis())
        {
            playingId=i;
            on_pBtnPlay_clicked();
            initListItem();
            ui->musiclist->setCurrentRow(i);
            ui->musiclist->item(i)->setIcon(QIcon(":/img/images/player_play.png"));
            return;
        }
    }
    ui->pPlayerSlider->setValue(0);
    //取出歌曲的名字：
    int len=playingfile.length();
    int pos=playingfile.lastIndexOf("/");
    QString itemtext=playingfile.right(len-pos-1);
    on_pBtnPlay_clicked();
    //设置歌曲条目的属性
    QListWidgetItem *item=new QListWidgetItem(itemtext);
    //item->setToolTip("playingfile");
    item->setWhatsThis(playingfile);
    item->setBackgroundColor("#EDEDED");
    item->setIcon(QIcon(":/img/images/player_play.png"));
    ui->musiclist->addItem(item);
    ui->musiclist->setCurrentItem(item);
    playingId=ui->musiclist->currentRow();
    initListItem();
}

void DWplayer::on_pBtnPlay_clicked()
{
    if(isPlaying==false)    //没有歌曲播放时
    {
        //调用mplayer播放歌曲
        QStringList args;
        args<<"-af";
        args<<"volume=-25";
        args<<"-slave";
        args<<"-quiet";
        args<<playingfile;
        playerProcess.start("mplayer",args);
        ui->pBtnPlay->setIcon(QIcon(":/img/images/player_pause.png"));
        ui->pBtnPlay->setToolTip("Pause");
        isPlaying=true;
        isSwitch=false;
        playerProcess.write("get_time_length\n");
        timer.start(1000);
        playerProcess.write("get_property volume\n");
        return;
    }

    else
    {
        if(isSwitch==true)            //有歌曲正在播放并且处于歌曲切换状态
        {
            //播放新的歌曲QString str=item->whatsThis();
            QStringList myList=playingfile.split(" ");
            QString playingfileTmp=myList.join("\\ ");
            QString cmd="loadfile "+playingfileTmp+"\n";
            playerProcess.write(cmd.toLatin1().data());
            isSwitch=false;
            if(ui->pBtnPlay->toolTip()=="Play")
            {
                ui->pBtnPlay->setIcon(QIcon(":/img/images/player_pause.png"));
                ui->pBtnPlay->setToolTip("Pause");
            }
            playerProcess.write("get_time_length\n");
            timer.start(1000);
            playerProcess.write("get_property volume\n");
            return;
        }
        playerProcess.write("pause\n");
        if(ui->pBtnPlay->toolTip()=="Pause")   //歌曲正在播放，按下暂停按钮
         {
            ui->pBtnPlay->setIcon(QIcon(":/img/images/player_play.png"));
            ui->pBtnPlay->setToolTip("Play");
            timer.stop();
        }
        else                                   //歌曲已暂停，按下播放按钮
        {
            ui->pBtnPlay->setIcon(QIcon(":/img/images/player_pause.png"));
            ui->pBtnPlay->setToolTip("Pause");
            timer.start(1000);
        }

    }

}

void DWplayer::initListItem()
{
    //将音乐列表中未播放歌曲前的图标置空
    for(int i=0;i<ui->musiclist->count();i++)
    {
        if(playingId==i)
        {
            continue;
        }
        ui->musiclist->item(i)->setIcon(QIcon());
    }
}

void DWplayer::getDoubleclicked(QListWidgetItem *item)
{
    item->setIcon(QIcon(":/img/images/player_play.png"));
    playingfile=item->whatsThis();
    isSwitch=true;
    on_pBtnPlay_clicked();
    playingId=ui->musiclist->currentRow();
    initListItem();
}

void DWplayer::readFromStout()
{
    while(playerProcess.canReadLine())                    //可以读取一行数据
    {
        int s=0;
        int e=0;
        QString str(playerProcess.readLine());            //读取一行数据到str
        if(str.startsWith("ANS_LENGTH="))
        {
            str=str.trimmed();                            //移除字符串两端的空白字符
            s=str.indexOf("=");
            e=str.indexOf(".");
            QString sec=str.mid(s+1,(e-s-1));
            ui->pPlayerSlider->setMaximum((sec.toInt()));  //设置播放进度条满值
            QTime totalTime((sec.toInt()/3600)%60,(sec.toInt()/60)%60,sec.toInt()%60);
            QString total=totalTime.toString("hh:mm:ss");
            ui->timesum->setText(total);
        }
        if(str.startsWith("ANS_TIME_POSITION="))
        {
            str=str.trimmed();                            //移除字符串两端的空白字符
            s=str.indexOf("=");
            e=str.indexOf(".");
            QString sec=str.mid(s+1,(e-s-1));
            int t=sec.toInt();
            ui->pPlayerSlider->setValue(t);
            QTime curTime((t/3600)%60,(t/60)%60,t%60);
            QString cur=curTime.toString("hh:mm:ss");
            ui->time->setText(cur);
        }
        if(str.startsWith("ANS_volume="))
        {
            str=str.trimmed();                            //移除字符串两端的空白字符
            s=str.indexOf("=");
            e=str.indexOf(".");
            QString sec=str.mid(s+1,(e-s-1));
            int t=sec.toInt();
            ui->voiceSlider->setValue(t);
        }
    }
}

void DWplayer::updateSlider(int pos)
{
    QString cmd="seek "+QString::number(pos)+" 2\n";
    playerProcess.write(cmd.toLatin1().data());
}

void DWplayer::updateSlider()
{
    playerProcess.write("get_time_pos\n");
}

void DWplayer::updateVol(int pos)
{
    QString cmd="volume "+QString::number(pos)+" 1\n";
    playerProcess.write(cmd.toLatin1().data());
    //cmd="osd_show_text "+QString("Volume:")+QString::number(pos+1)+"/"+"100\n";
    //playerProcess.write(cmd.toLatin1().data());
}

void DWplayer::on_pBtnVol_clicked()
{
    static int i=0;
    if(i==0)
    {
        ui->pBtnVol->setIcon(QIcon(":/img/images/volume_mute.png"));
        ui->pBtnVol->setToolTip("Unmute");
        i=1;
        playerProcess.write("mute 1\n");
        return;
    }
    if(i==1)
    {
        ui->pBtnVol->setIcon(QIcon(":/img/images/volume.png"));
        ui->pBtnVol->setToolTip("Mute");
        i=0;
        playerProcess.write("mute 0\n");
    }
}
/*
void DWplayer::on_pBtnStop_clicked()
{b
    playerProcess.write("q\n");
    isPlaying=false;
    ui->musiclist->item(playingId)->setIcon(QIcon());
    ui->pBtnPlay->setIcon(QIcon(":/img/images/player_play.png"));
}
*/
void DWplayer::on_pBtnUp_clicked()
{
    ui->musiclist->item(playingId)->setIcon(QIcon());
    if(playingId==0)
    {
        playingId=ui->musiclist->count();
    }
    playingfile=ui->musiclist->item(--playingId)->whatsThis();
    isSwitch=true;
    isPlaying=true;
    on_pBtnPlay_clicked();
    ui->musiclist->setCurrentItem(ui->musiclist->item(playingId));
    ui->musiclist->item(playingId)->setIcon(QIcon(":/img/images/player_play.png"));
    if(isRepeatedOne)
    {
        playerProcess.write("loop 1\n");
    }
}

void DWplayer::on_pBtnDown_clicked()
{
    ui->musiclist->item(playingId)->setIcon(QIcon());
    if(playingId==ui->musiclist->count()-1)
    {
        playingId=-1;
    }
    playingfile=ui->musiclist->item(++playingId)->whatsThis();
    isSwitch=true;
    isPlaying=true;
    on_pBtnPlay_clicked();
    ui->musiclist->setCurrentItem(ui->musiclist->item(playingId));
    ui->musiclist->item(playingId)->setIcon(QIcon(":/img/images/player_play.png"));
    if(isRepeatedOne)
    {
        playerProcess.write("loop 1\n");
    }
}

void DWplayer::on_pBtnFaster_clicked()
{
    playerProcess.write("speed_mult 2\n");
}

void DWplayer::on_pBtnSlower_clicked()
{
    playerProcess.write("speed_mult 0.5\n");
}

void DWplayer::on_pBtnMode_clicked()
{
    static int i=0;
    if(i==0)
    {
        ui->pBtnMode->setIcon(QIcon(":/img/images/repeat_one.png"));
        ui->pBtnMode->setToolTip("repeated all");
        i=1;
        playerProcess.write("loop 1\n");
        isRepeatedALL=false;
        isRepeatedOne=true;
        return;
    }
    if(i==1)
    {
        ui->pBtnMode->setIcon(QIcon(":/img/images/repeat_all.png"));
        ui->pBtnMode->setToolTip("repeated one");
        i=0;
        playerProcess.write("loop -1\n");
        isRepeatedALL=true;
        isRepeatedOne=false;
        return;
    }
}

void DWplayer::getPlayerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    isPlaying=false;
    ui->pPlayerSlider->setValue(0);
    if(isRepeatedALL)
    {
        playingId=(++playingId)%ui->musiclist->count();
        playingfile=ui->musiclist->item(playingId)->whatsThis();
        ui->musiclist->setCurrentRow(playingId);
        ui->musiclist->item(playingId)->setIcon(QIcon(":/img/images/player_play.png"));
        initListItem();
        on_pBtnPlay_clicked();
    }
}
