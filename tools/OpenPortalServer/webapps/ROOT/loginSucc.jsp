<%@ page language="java" import="java.util.*" pageEncoding="UTF-8"%>
<%
String path = request.getContextPath();
String basePath = request.getScheme()+"://"+request.getServerName()+":"+request.getServerPort()+path+"/";
%>
<%
String portalPath = request.getScheme()+"://"+request.getServerName()+":"+request.getServerPort()+"/";
String radiusPath = request.getScheme()+"://"+request.getServerName()+":"+1817+"/login";
%>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<base href="<%=basePath%>"/>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1" />
    <title>用户登录成功</title>
    <meta http-equiv="pragma" content="no-cache"/>
    <meta http-equiv="cache-control" content="no-cache"/>
    <meta http-equiv="expires" content="0"/>
    <meta http-equiv="keywords" content="Open Portal"/>
    <meta http-equiv="description" content="Open Portal info"/>
    <link type="text/css" href="css/index.css" rel="stylesheet" />
    <script type="text/javascript">

function _submit() {
	/*
	1. 得到img元素
	2. 修改其src为/day11_3/VerifyCodeServlet
	*/
	document.getElementById("msg").innerHTML = "正在请求认证，请稍等....";
	document.getElementById("loginoffSubmit").disabled=true;
	return true;
}
</script>
</head>
 <%
    String username=(String)session.getAttribute("username");
    String password=(String)session.getAttribute("password");
    String ip=(String)session.getAttribute("ip");
    String message="";
    String msg=(String)request.getAttribute("msg");
    if(msg!=null){
    	message=msg;
    }
    
    if(username==null){
    	request.setAttribute("msg", "您还没有登录，请先登录！");
    	request.getRequestDispatcher("/index.jsp").forward(request, response);
    	return;
    }
    else{
    %>
<body>
    <div id="page-content">
        <div id="login-page">
            <div id="logo">
                <a href="<%=basePath%>"><img alt="LaterThis" src="images/logo.png" /></a>
            </div>
           <form id="loginForm" action="<%=path%>/LoginOut" method="post"  onsubmit="_submit()">
              <div id="success-login">
              <p>
                        <label style="text-align: center ;"><font color="red"><b id="msg"><%=message%></b></font></label> <br/>
                    </p>
			        <p>
			          <label>您已登录成功，可以连接网络，请不要关闭该窗口！！欢迎您：</label><span id="success-user" class="id-note"><font color="red"><b><%=username%></b></font></span>
			          <br/>
			           <label>IP地址：</LABEL><span id="success-user" class="id-note"><font color="red"><b><%=ip%></b></font></span>
			           <br/>
			        </p>
			        <p>
			          <input id="loginoffSubmit" class="button" type="submit" value="退出" name="logoff" />  
			        </p>
			      </div>
              </form>
              <form id="Form" action="<%=radiusPath%>" method="post" target="_blank">
              <div id="success-login">
              <p>
               <input name="username" type="hidden" value="<%=username%>" />
               <input name="password" type="hidden" value="<%=password%>" />
			          <input id="submit" class="button" type="submit" value="点击查询用户信息" name="submit" />  
			        </p>
			      </div>
              </form>
                <p id="signup">
                   Copyright &copy; 2014 - 2015 <a href="<%=basePath%>">OpenPortal Server-李硕</a>.  All Rights Reserved.
               </p>
        </div>
    </div>
</body>
<%} %>
</html>