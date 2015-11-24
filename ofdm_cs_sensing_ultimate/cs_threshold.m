function y = cs_threshold(x, N, fs, M, fc, BW)
%%%%% 参量说明 %%%%%
% x -- 信号
% N -- 循环谱检测采样长度，必须小于等于信号序列长度
% fs -- 采样频率, 检测带宽为-fs/2至fs/2
% M -- 平滑点数, 时间分辨率*频率分辨率=M
% BW -- 信号带宽
%%%%%%%%%%%%%%%%%%%

%%% 参数 %%% 
win = 'hamming'; % 平滑窗类型

d_alpha = fs/N; % 1/时间分辨率=循环频率分辨率
alpha = 0:d_alpha:fs; % 循环频率, 分辨率=1/时间分辨率
a_len = length(alpha); % 循环频率取样个数

f_len = floor(N/M-1)+1; % 最大平滑窗个数, 即频率采样个数
f = -(fs/2-d_alpha*floor(M/2)) + d_alpha*M*(0:f_len-1); % 频率采样点位置

S = zeros(a_len, f_len); % 初始相关功率谱
i = 1; 

fl = ceil((2*fc-BW)/d_alpha)+1; % 所需检测a的下限序号
rt = floor((2*fc+BW)/d_alpha)+1; % 所需检测a的上限序号
peak_a = fl:rt; % a的检测范围
peak_f = (1:ceil(BW/d_alpha/M))+floor(f_len/2-floor(BW/d_alpha/M)/2); % f的最大检测范围
noise_n = 0; % 噪声点数初始

%%% 信号fft变换 %%%
X = fftshift(fft(x(1:N))); 
X = X';

%%% 遍历循环频率取值 %%%
for alfa = alpha   
    
    if i >= fl && i <= rt % 仅检测所需a范围
        
        interval_f_N = round(alfa/d_alpha); % 循环频率所对应的频谱序列序号
        f_N = floor((N-interval_f_N-M)/M)+1; % 平滑窗的个数
    
        %%% 生成平滑窗函数 %%%
        g = feval(win, M); 
        window_M = g(:, ones(f_N,1));
    
        %%% 频域序列平滑模板 %%%
        t = 1:M*f_N;
        t = reshape(t, M, f_N);
    
        %%% 计算X1,X2 %%%
        X1 = X(t).*window_M;
        X2 = X(t+interval_f_N).*window_M; 

        %%% 计算谱相关 %%%
        St = conj(X1).*X2;
        St = mean(St, 1); % 平滑平均
    
        noise_n = noise_n + length(St); % 累计噪声点
        S(i, floor((f_len-f_N)/2)+(1:f_N)) = St/N; % 将结果平移至序列中央以便作图
    end
    
    i = i+1; 
end
%%% 遍历循环频率取值结束 %%%
 
%%% 循环功率谱作图 %%%
%mesh(f, alpha, abs(S)); 
%axis tight;
%title('BPSK-OFDM');
%xlabel('f'); ylabel('a');

peak = S(peak_a, peak_f); % 取出峰值区域
S(peak_a, peak_f) = 0; 
noise = S(peak_a,:); % 取出噪声区域 

p = sort(abs(peak(:)),'descend'); % 排序峰值区域
peak_mean = mean(p(1:ceil(length(p)*0.1))); % 前10%点平均

noise_n = noise_n-length(peak(:)); % 实际噪声点数
n = sort(abs(noise(:)),'descend'); % 排序噪声区域
noise_mean = mean(n(1:noise_n)); % 噪声平均

y = peak_mean/noise_mean; % 计算比值