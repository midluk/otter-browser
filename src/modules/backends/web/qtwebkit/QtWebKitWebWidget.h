/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2016 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2015 - 2016 Jan Bajer aka bajasoft <jbajer@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#ifndef OTTER_QTWEBKITWEBWIDGET_H
#define OTTER_QTWEBKITWEBWIDGET_H

#include "../../../../ui/WebWidget.h"

#include <QtCore/QQueue>
#include <QtNetwork/QNetworkReply>
#include <QtWebKitWidgets/QWebHitTestResult>
#include <QtWebKitWidgets/QWebInspector>
#include <QtWebKitWidgets/QWebPage>
#include <QtWebKitWidgets/QWebView>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QToolButton>

namespace Otter
{

class ContentsDialog;
class QtWebKitInspector;
class QtWebKitNetworkManager;
class QtWebKitWebBackend;
class QtWebKitPage;
class QtWebKitPluginFactory;
class SourceViewerWebWidget;

class QtWebKitWebWidget : public WebWidget
{
	Q_OBJECT

public:
	~QtWebKitWebWidget();

	void search(const QString &query, const QString &searchEngine);
	void print(QPrinter *printer);
	WebWidget* clone(bool cloneHistory = true, bool isPrivate = false);
	Action* getAction(int identifier);
	QString getDefaultCharacterEncoding() const;
	QString getTitle() const;
	QString getActiveStyleSheet() const;
	QString getSelectedText() const;
	QStringList getBlockedElements() const;
	QUrl getUrl() const;
	QIcon getIcon() const;
	QPixmap getThumbnail();
	QPoint getScrollPosition() const;
	QRect getProgressBarGeometry() const;
	WebWidget::SslInformation getSslInformation() const;
	WindowHistoryInformation getHistory() const;
	HitTestResult getHitTestResult(const QPoint &position);
	QStringList getStyleSheets() const;
	QList<LinkUrl> getFeeds() const;
	QList<LinkUrl> getSearchEngines() const;
	QHash<QByteArray, QByteArray> getHeaders() const;
	QVariantHash getStatistics() const;
	WindowsManager::ContentStates getContentState() const;
	WindowsManager::LoadingState getLoadingState() const;
	int getZoom() const;
	bool hasSelection() const;
	bool isPrivate() const;
	bool findInPage(const QString &text, FindFlags flags = NoFlagsFind);
	bool eventFilter(QObject *object, QEvent *event);

public slots:
	void clearOptions();
	void clearSelection();
	void goToHistoryIndex(int index);
	void removeHistoryIndex(int index, bool purge = false);
	void triggerAction(int identifier, const QVariantMap &parameters = QVariantMap());
	void setActiveStyleSheet(const QString &styleSheet);
	void setPermission(const QString &key, const QUrl &url, PermissionPolicies policies);
	void setOption(const QString &key, const QVariant &value);
	void setScrollPosition(const QPoint &position);
	void setHistory(const WindowHistoryInformation &history);
	void setZoom(int zoom);
	void setUrl(const QUrl &url, bool typed = true);

protected:
	enum HistoryEntryData
	{
		IdentifierEntryData = 0,
		ZoomEntryData = 1,
		PositionEntryData = 2
	};

	explicit QtWebKitWebWidget(bool isPrivate, WebBackend *backend, QtWebKitNetworkManager *networkManager = NULL, ContentsWidget *parent = NULL);

	void timerEvent(QTimerEvent *event);
	void focusInEvent(QFocusEvent *event);
	void clearPluginToken();
	void resetSpellCheck(QWebElement element);
	void openRequest(const QUrl &url, QNetworkAccessManager::Operation operation, QIODevice *outgoingData);
	void openFormRequest(const QUrl &url, QNetworkAccessManager::Operation operation, QIODevice *outgoingData);
	void pasteText(const QString &text);
	void startDelayedTransfer(Transfer *transfer);
	void handleHistory();
#ifdef OTTER_ENABLE_QTWEBKITNG
	void setHistory(const QVariantMap &history);
#else
	void setHistory(QDataStream &stream);
#endif
	void setOptions(const QVariantHash &options);
	QWebPage* getPage();
	QString getPasswordToken() const;
	QString getPluginToken() const;
	int getAmountOfNotLoadedPlugins() const;
	bool canLoadPlugins() const;
	bool canGoBack() const;
	bool canGoForward() const;
	bool canShowContextMenu(const QPoint &position) const;
	bool canViewSource() const;
	bool isNavigating() const;
	bool isScrollBar(const QPoint &position) const;

protected slots:
	void optionChanged(const QString &option, const QVariant &value);
	void navigating(const QUrl &url, QWebFrame *frame, QWebPage::NavigationType type);
	void pageLoadStarted();
	void pageLoadFinished();
	void downloadFile(const QNetworkRequest &request);
	void downloadFile(QNetworkReply *reply);
	void saveState(QWebFrame *frame, QWebHistoryItem *item);
	void restoreState(QWebFrame *frame);
	void linkHovered(const QString &link);
	void openFormRequest();
	void viewSourceReplyFinished(QNetworkReply::NetworkError error = QNetworkReply::NoError);
	void handlePrintRequest(QWebFrame *frame);
	void handleWindowCloseRequest();
	void handlePermissionRequest(QWebFrame *frame, QWebPage::Feature feature);
	void handlePermissionCancel(QWebFrame *frame, QWebPage::Feature feature);
	void notifyTitleChanged();
	void notifyUrlChanged(const QUrl &url);
	void notifyIconChanged();
	void notifyPermissionRequested(QWebFrame *frame, QWebPage::Feature feature, bool cancel);
	void notifyAddPasswordRequested(const PasswordsManager::PasswordInformation &password);
	void notifyContentStateChanged();
	void updateUndoText(const QString &text);
	void updateRedoText(const QString &text);
	void updateOptions(const QUrl &url);

private:
	QWebView *m_webView;
	QtWebKitPage *m_page;
	QtWebKitPluginFactory *m_pluginFactory;
	QtWebKitInspector *m_inspector;
	QtWebKitNetworkManager *m_networkManager;
	QSplitter *m_splitter;
	QString m_passwordToken;
	QString m_pluginToken;
	QPixmap m_thumbnail;
	QUrl m_formRequestUrl;
	QByteArray m_formRequestBody;
	QQueue<Transfer*> m_transfers;
	QHash<QNetworkReply*, QPointer<SourceViewerWebWidget> > m_viewSourceReplies;
	QNetworkAccessManager::Operation m_formRequestOperation;
	WindowsManager::LoadingState m_loadingState;
	int m_transfersTimer;
	bool m_canLoadPlugins;
	bool m_isTyped;
	bool m_isNavigating;

signals:
	void widgetActivated(WebWidget *widget);

friend class QtWebKitNetworkManager;
friend class QtWebKitPluginFactory;
friend class QtWebKitWebBackend;
friend class QtWebKitPage;
};

}

#endif
