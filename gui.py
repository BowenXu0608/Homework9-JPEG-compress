# -*- coding: gbk -*-
import sys
import subprocess
import shutil
import sys
import subprocess
import shutil
import os
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QLabel, QPushButton, QFileDialog, QMessageBox
from PyQt5.QtGui import QPixmap
from PyQt5.QtCore import Qt

class JpegCompressorApp(QWidget):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.exe_path = r"x64\Release\homework_week_9.exe"
        self.temp_output = "temp_preview.jpg"

    def initUI(self):
        self.setWindowTitle('自定义 BMP 转 JPEG 压缩工具')
        self.resize(600, 700)
        # 允许拖拽
        self.setAcceptDrops(True)

        layout = QVBoxLayout()

        # 预览与拖拽区域
        self.label = QLabel('将 BMP 图片拖拽到这里\n或点击下方按钮选择文件', self)
        self.label.setAlignment(Qt.AlignCenter)
        self.label.setStyleSheet("QLabel { border: 2px dashed #aaaaaa; font-size: 18px; color: #555555; background-color: #f9f9f9;}")
        self.label.setMinimumHeight(500)
        layout.addWidget(self.label)

        # 按钮组件
        self.btn_select = QPushButton('选择 BMP 文件', self)
        self.btn_select.setMinimumHeight(40)
        self.btn_select.clicked.connect(self.select_file)
        layout.addWidget(self.btn_select)

        self.btn_save = QPushButton('下载 / 另存为 JPEG', self)
        self.btn_save.setMinimumHeight(40)
        self.btn_save.setEnabled(False) # 处理前禁用保存按钮
        self.btn_save.clicked.connect(self.save_file)
        layout.addWidget(self.btn_save)

        self.setLayout(layout)

    # 处理拖入事件
    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls():
            event.accept()
        else:
            event.ignore()

    # 处理松开拖拽事件
    def dropEvent(self, event):
        files = [u.toLocalFile() for u in event.mimeData().urls()]
        if files:
            self.process_image(files)

    def select_file(self):
        file_path, _ = QFileDialog.getOpenFileName(self, "选择图片", "", "BMP Files (*.bmp)")
        if file_path:
            self.process_image(file_path)

    def process_image(self, input_path):
        if not input_path.lower().endswith('.bmp'):
            QMessageBox.warning(self, "格式错误", "请上传 BMP 格式的图片！")
            return

        if not os.path.exists(self.exe_path):
            QMessageBox.critical(self, "缺失核心组件", f"找不到 {self.exe_path}，请确保你的 C++ 程序已编译并放在同级目录。")
            return

        self.label.setText("正在执行 C++ 算法压缩中...")
        QApplication.processEvents()

        try:
            # 调用你的 C++ 算法可执行文件，传入输入和临时输出路径
            subprocess.run([self.exe_path, input_path, self.temp_output], check=True)

            # 在界面上显示压缩后的 JPEG 预览
            pixmap = QPixmap(self.temp_output)
            self.label.setPixmap(pixmap.scaled(self.label.size(), Qt.KeepAspectRatio, Qt.SmoothTransformation))
            
            # 激活保存按钮
            self.btn_save.setEnabled(True)
            self.btn_save.setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;")
        except Exception as e:
            QMessageBox.critical(self, "错误", f"图片处理失败:\n{str(e)}")
            self.label.setText('将 BMP 图片拖拽到这里\n或点击下方按钮选择文件')

    def save_file(self):
        if os.path.exists(self.temp_output):
            save_path, _ = QFileDialog.getSaveFileName(self, "保存 JPEG 图片", "compressed_image.jpg", "JPEG Files (*.jpg)")
            if save_path:
                # 将临时预览文件复制到用户指定的下载路径
                shutil.copy(self.temp_output, save_path)
                QMessageBox.information(self, "成功", f"图片已成功保存至:\n{save_path}")

    def closeEvent(self, event):
        # 退出时清理临时文件
        if os.path.exists(self.temp_output):
            os.remove(self.temp_output)
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = JpegCompressorApp()
    ex.show()
    sys.exit(app.exec_())


