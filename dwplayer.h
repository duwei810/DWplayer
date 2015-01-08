#ifndef DWPLAYER_H
#define DWPLAYER_H

#include <QWidget>
#include<QListWidget>
#include<QProcess>
#include<QListWidgetItem>
#include<QTimer>
#include<QTime>
#include<QFileDialog>
#include<QTextCodec>

namespace Ui {
class DWplayer;
}

class DWplayer : public QWidget
{
    Q_OBJECT

public:
    explicit DWplayer(QWidget *parent = 0);
    ~DWplayer();

private slots:
    void on_pBtnOpen_clicked(); //播放/暂停按钮的单击槽函数

    void initListItem(); //初始化音乐列表的槽函数，将未播放的歌曲前的图片置空

    void on_pBtnPlay_clicked(); //打开文件夹的槽函数

    void getDoubleclicked(QListWidgetItem *); //双击音乐列表里的歌曲的槽函数

    void readFromStout(); //解析MPlayer，得到播放总时间,当前播放时间和音量值

    void updateSlider(); //更新播放进度

    void updateSlider(int); //拖动进度条位置更新播放位置

    void updateVol(int); //拖动进度条位置调节音量

    void on_pBtnVol_clicked(); //开\关音量

    void on_pBtnUp_clicked(); //上一首

    void on_pBtnDown_clicked(); //下一首

    void on_pBtnFaster_clicked(); //倍速播放

    void on_pBtnSlower_clicked(); //慢速播放

    void on_pBtnMode_clicked(); //切换播放模式：单曲循环/顺序播放

    void getPlayerFinished(int,QProcess::ExitStatus); //结束播放进程

private:
    Ui::DWplayer *ui;
    QProcess playerProcess; //控制mplayer进程
    QString playingfile; //歌曲的绝对路径
    bool isPlaying; //歌曲播放状态
    bool isSwitch;  //歌曲切换状态
    quint16 playingId; //播放歌曲的索引号
    QTimer timer; //更新播放位置
    bool isRepeatedALL; //是否顺序播放
    bool isRepeatedOne; //是否单曲循环
};

#endif // DWPLAYER_H
