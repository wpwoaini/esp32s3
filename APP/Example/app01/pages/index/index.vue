<template>
	<view class="content">
		<image class="logo" src="/static/logo.png"></image>
		<view class="text-area">
			<text class="title">{{token}}</text>
		</view>
	</view>
</template>

<script>
	const {
		createCommonToken
	} = require('@/key.js');
	export default {
		data() {
			return {
				title: 'Hello',
				token: '',
			}
		},

		onLoad() {
			const params = {
				author_key: "OSiE2fPgATAh4PaYLUt7GbZtdu9y75qYM3ackBgaBXkLV9qZYbgXzEBzqD/X70CJ", // 替换为你的产品级别 accessKey
				version: "2022-05-01",
				user_id: "452540", // 如果是用户鉴权，填写用户ID，但我们当前用产品鉴权
				product_id: "3OYc99Dt2k ", // 你的产品ID
				res: "products/3OYc99Dt2k ", // 明确指定 res 为产品鉴权
				expirationTimeInSeconds: 24 * 3600 // Token 有效期设为 24 小时
			};
			this.token = createCommonToken(params);
		},
		methods: {

		}
	}
</script>

<style>
	.content {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
	}

	.logo {
		height: 200rpx;
		width: 200rpx;
		margin-top: 200rpx;
		margin-left: auto;
		margin-right: auto;
		margin-bottom: 50rpx;
	}

	.text-area {
		display: flex;
		justify-content: center;
	}

	.title {
		font-size: 36rpx;
		color: #8f8f94;
	}
</style>