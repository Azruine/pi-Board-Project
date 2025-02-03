import QtQuick 2.15
import QtQuick.Window 2.15

Window {
    visible: true
    width: 800
    height: 480
    title: qsTr("Weather Dashboard")

    Rectangle {
        anchors.fill: parent
        color: "#2E3B4E"

        Column {
            anchors.centerIn: parent
            spacing: 30

            // 현재 시각 표시 (동적으로 갱신되도록 Timer 사용)
            Text {
                id: currentTimeText
                text: Qt.formatTime(new Date(), "hh:mm:ss")
                font.pixelSize: 40
                color: "lightblue"
                horizontalAlignment: Text.AlignHCenter
            }
            Timer {
                interval: 1000; running: true; repeat: true
                onTriggered: currentTimeText.text = Qt.formatTime(new Date(), "hh:mm:ss")
            }

            // 오늘 날씨와 향후 6시간 기온을 수평으로 배치
            Row {
                spacing: 40
                anchors.horizontalCenter: parent.horizontalCenter

                // 오늘 날씨 정보 (왼쪽 컬럼)
                Column {
                    spacing: 5
                    Text {
                        text: todayWeatherData.temperature
                        font.pixelSize: 32
                        font.bold: true
                        color: "lightblue"
                    }
                    Text {
                        text: todayWeatherData.condition
                        font.pixelSize: 28
                        color: "white"
                    }
                    Image {
                        width: 80    // 기존의 두 배 크기
                        height: 80
                        fillMode: Image.PreserveAspectFit
                        source: todayWeatherData.iconUrl
                    }
                }

                // 향후 6시간 기온 정보 (오른쪽 컬럼)
                Column {
                    spacing: 5
                    Text {
                        text: "향후 6시간"
                        font.pixelSize: 20
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Row {
                        spacing: 10
                        Repeater {
                            model: weatherManager.hourlyForecast
                            delegate: Column {
                                spacing: 2
                                Text {
                                    text: modelData.time  // "HH:mm"
                                    font.pixelSize: 14
                                    color: "white"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                Image {
                                    width: 32
                                    height: 32
                                    fillMode: Image.PreserveAspectFit
                                    source: modelData.icon
                                }
                                Text {
                                    text: modelData.temp
                                    font.pixelSize: 14
                                    color: "lightgray"
                                    horizontalAlignment: Text.AlignHCenter
                                }
                            }
                        }
                    }
                }
            }

            // 7일 예보 영역 (아래 배치)
            Row {
                id: forecastRow
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 15
                Repeater {
                    model: weeklyWeatherModel
                    delegate: Column {
                        spacing: 5
                        Text {
                            text: day
                            font.pixelSize: 16
                            color: "white"
                        }
                        Image {
                            width: 32
                            height: 32
                            fillMode: Image.PreserveAspectFit
                            source: iconUrl
                        }
                        Text {
                            text: maxTemperature
                            font.pixelSize: 14
                            color: "lightgray"
                        }
                        Text {
                            text: minTemperature
                            font.pixelSize: 14
                            color: "lightgray"
                        }
                    }
                }
            }
        }
    }
}
