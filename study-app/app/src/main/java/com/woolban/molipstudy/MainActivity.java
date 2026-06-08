package com.woolban.molipstudy;

import android.annotation.SuppressLint;
import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.webkit.CookieManager;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceError;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.ProgressBar;

import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;

/**
 * "우리반 몰입 스터디" WebView 래퍼.
 * 기존 Google Apps Script 웹앱을 그대로 로드한다.
 * - DOM Storage(localStorage) 활성화 → 학번 자동 로그인 유지
 * - JavaScript 활성화 → google.script.run / 서명 캔버스 동작
 */
public class MainActivity extends AppCompatActivity {

    private WebView webView;
    private ProgressBar progressBar;

    @SuppressLint("SetJavaScriptEnabled")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        webView = findViewById(R.id.webview);
        progressBar = findViewById(R.id.progress);

        WebSettings s = webView.getSettings();
        s.setJavaScriptEnabled(true);
        s.setDomStorageEnabled(true);            // localStorage (자동 로그인)
        s.setDatabaseEnabled(true);
        s.setJavaScriptCanOpenWindowsAutomatically(true);
        s.setMixedContentMode(WebSettings.MIXED_CONTENT_COMPATIBILITY_MODE);
        s.setCacheMode(WebSettings.LOAD_DEFAULT);
        s.setMediaPlaybackRequiresUserGesture(false);
        s.setUseWideViewPort(true);
        s.setLoadWithOverviewMode(true);
        s.setSupportZoom(false);
        s.setBuiltInZoomControls(false);
        // 사용자 에이전트는 기본값 유지 (커스텀 UA는 구글이 빈 화면을 줄 수 있음)

        webView.setBackgroundColor(Color.parseColor("#F8FAFC"));

        // Apps Script는 실제 화면을 cross-origin iframe(script.googleusercontent.com)으로
        // 렌더링한다. 서드파티 쿠키를 허용하지 않으면 흰 화면만 보인다.
        CookieManager cookieManager = CookieManager.getInstance();
        cookieManager.setAcceptCookie(true);
        cookieManager.setAcceptThirdPartyCookies(webView, true);

        webView.setWebChromeClient(new WebChromeClient() {
            @Override
            public void onProgressChanged(WebView view, int newProgress) {
                progressBar.setProgress(newProgress);
                progressBar.setVisibility(newProgress >= 100 ? View.GONE : View.VISIBLE);
            }
        });

        webView.setWebViewClient(new WebViewClient() {
            @Override
            public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
                // 모든 페이지를 WebView 내부에서 로드 (구글 스크립트/콘텐츠 도메인 포함)
                return false;
            }

            // 메인 페이지 로드 실패 시 흰 화면 대신 원인 안내를 보여준다.
            @Override
            public void onReceivedHttpError(WebView view, WebResourceRequest request, WebResourceResponse response) {
                if (request.isForMainFrame()) {
                    showErrorPage(view, "HTTP " + response.getStatusCode()
                            + " — 웹앱 접근이 거부되었습니다.\n\n웹앱 배포 권한이 '모든 사용자(Anyone)'로 설정되어 있는지 확인해주세요."
                            + " (앱 내부 WebView는 Google 로그인을 공유하지 않으므로, 로그인 없이 열리는 공개 배포가 필요합니다.)");
                }
            }

            @Override
            public void onReceivedError(WebView view, WebResourceRequest request, WebResourceError error) {
                if (request.isForMainFrame()) {
                    showErrorPage(view, "페이지를 불러오지 못했습니다.\n\n인터넷 연결을 확인해주세요.\n(" + error.getDescription() + ")");
                }
            }
        });

        if (savedInstanceState != null) {
            webView.restoreState(savedInstanceState);
        } else {
            webView.loadUrl(getString(R.string.app_url));
        }

        // 기기 뒤로가기: 웹 히스토리가 있으면 그쪽 우선
        getOnBackPressedDispatcher().addCallback(this, new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                if (webView.canGoBack()) {
                    webView.goBack();
                } else {
                    setEnabled(false);
                    getOnBackPressedDispatcher().onBackPressed();
                }
            }
        });
    }

    private void showErrorPage(WebView view, String message) {
        String safe = message.replace("<", "&lt;").replace("\n", "<br>");
        String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
                + "</head><body style=\"font-family:sans-serif;padding:28px;color:#334155;line-height:1.7\">"
                + "<h3 style='color:#dc2626'>화면을 열 수 없습니다</h3>"
                + "<p>" + safe + "</p>"
                + "</body></html>";
        view.loadDataWithBaseURL(null, html, "text/html", "UTF-8", null);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        webView.saveState(outState);
    }
}
