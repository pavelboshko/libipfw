package ru.securitycode.ipfw

import android.content.Context

fun readAssetsFile(fname : String, ctx : Context): ByteArray {
    return ctx.assets.open(fname).readBytes()
}